import struct

import lldb
from lldbsuite.test.decorators import skipIfXmlSupportMissing
from lldbsuite.test.gdbclientutils import MockGDBServerResponder
from lldbsuite.test.lldbgdbclient import GDBRemoteTestBase


class SBPFResponder(MockGDBServerResponder):
    def __init__(self, frame_pointers):
        super().__init__()
        # Identical PCs model recursion through the same call site.
        self.frames = [(0x100, fp) for fp in frame_pointers]
        self.frames[0] = (0x108, frame_pointers[0])

    def _respond_impl(self, packet):
        # Step past the instruction at a recursive return address before
        # continuing to the next return.
        if packet == "s":
            pc, fp = self.frames[0]
            self.frames[0] = (pc + 8, fp)
            return self.haltReason()
        return super()._respond_impl(packet)

    def qC(self):
        return "QC1"

    def qfThreadInfo(self):
        return "m1"

    def haltReason(self):
        return "T05thread:1;"

    def qXferRead(self, obj, annex, offset, length):
        if obj != "features" or annex != "target.xml":
            return None, False
        registers = []
        for index in range(12):
            generic = {10: ' generic="fp"', 11: ' generic="pc"'}.get(index, "")
            registers.append(
                f'<reg name="r{index}" bitsize="64" regnum="{index}"{generic}/>'
            )
        xml = (
            '<target><architecture>sbpf</architecture>'
            '<feature name="org.gnu.gdb.sbpf.cpu">'
            + "".join(registers)
            + "</feature></target>"
        )
        return xml[offset : offset + length], offset + length < len(xml)

    def readRegister(self, register):
        value = {10: self.frames[0][1], 11: self.frames[0][0]}.get(register, 0)
        return struct.pack("<Q", value).hex()

    def readRegisters(self):
        return "".join(self.readRegister(index) for index in range(12))

    def other(self, packet):
        if packet == "qSBPFCallStack:1":
            return b"".join(struct.pack("<QQ", *frame) for frame in self.frames).hex()
        return ""

    def setBreakpoint(self, packet):
        return "OK"

    def cont(self):
        self.frames.pop(0)
        return self.haltReason() if self.frames else "W00"


class TestSBPFCallStack(GDBRemoteTestBase):
    def check_frames(self, pointers):
        self.server.responder = SBPFResponder(pointers)
        target = self.dbg.CreateTargetWithFileAndTargetTriple("", "sbpf")
        self.assertTrue(target.IsValid())
        process = self.connect(target)
        thread = process.GetThreadAtIndex(0)
        for remaining in range(len(pointers), 0, -1):
            self.assertEqual(thread.GetNumFrames(), remaining)
            for index, fp in enumerate(pointers[-remaining:]):
                frame = thread.GetFrameAtIndex(index)
                self.assertEqual(
                    frame.GetPC(),
                    0x108 if remaining == len(pointers) and index == 0 else 0x100,
                )
                self.assertEqual(frame.GetFP(), fp)
                # A remote frame pointer must never masquerade as a DWARF CFA.
                self.assertEqual(frame.GetCFA(), lldb.LLDB_INVALID_ADDRESS)
            if remaining > 1:
                thread.StepOut()
                self.assertEqual(process.GetState(), lldb.eStateStopped)
        self.assertPacketLogReceived(["qSBPFCallStack:1"])

    @skipIfXmlSupportMissing
    def test_fixed_frames(self):
        self.check_frames([0x200006000, 0x200004000, 0x200002000])

    @skipIfXmlSupportMissing
    def test_dynamic_frames(self):
        self.check_frames([0x200003F80, 0x200003FC0, 0x200004000])

    @skipIfXmlSupportMissing
    def test_recursive_frames_with_same_pointer(self):
        self.check_frames([0x200004000] * 3)
