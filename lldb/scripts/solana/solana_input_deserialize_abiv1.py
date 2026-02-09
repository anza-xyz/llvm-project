"""
Solana Program Input Deserializer for LLDB (ABI v1)

Follows the deserialization logic from:
https://github.com/anza-xyz/solana-sdk/blob/0b0796150d3016e88b89a509db34dcf7e34afd3c/program-entrypoint/src/lib.rs#L464

Usage in LLDB:
    (lldb) command script import /path/to/solana_input_deserialize_abiv1.py
    (lldb) solana_input_deserialize_abiv1                          # default address
    (lldb) solana_input_deserialize_abiv1 0x400000000              # custom address
    (lldb) solana_input_deserialize_abiv1 0x400000000 10240        # custom max_permitted_data_increase
"""

import struct
from typing import Tuple

# Constants from solana-program-entrypoint
NON_DUP_MARKER = 0xFF
MAX_PERMITTED_DATA_INCREASE = 10 * 1024
BPF_ALIGN_OF_U128 = 8
PUBKEY_SIZE = 32
INPUT_BASE_ADDRESS = 0x400000000


def encode_base58(data: bytes) -> str:
    return b58encode(data).decode('ascii')


class MemoryReader:
    def __init__(self, process, base_address: int):
        self.process = process
        self.base = base_address

    def read(self, offset: int, size: int) -> bytes:
        import lldb
        err = lldb.SBError()
        data = self.process.ReadMemory(self.base + offset, size, err)
        if not err.Success():
            raise RuntimeError(f"Read failed at 0x{self.base + offset:x}: {err}")
        return data

    def u8(self, offset: int) -> int:
        return struct.unpack('<B', self.read(offset, 1))[0]

    def u32(self, offset: int) -> int:
        return struct.unpack('<I', self.read(offset, 4))[0]

    def u64(self, offset: int) -> int:
        return struct.unpack('<Q', self.read(offset, 8))[0]

    def pubkey(self, offset: int) -> str:
        return encode_base58(self.read(offset, PUBKEY_SIZE))


def align(offset: int) -> int:
    return (offset + BPF_ALIGN_OF_U128 - 1) & ~(BPF_ALIGN_OF_U128 - 1)


def format_hex(data: bytes, indent: str = "      ") -> str:
    """Format bytes as hex, 16 bytes per line."""
    lines = []
    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        hex_str = ' '.join(f'{b:02x}' for b in chunk)
        lines.append(f"{indent}{hex_str}")
    return '\n'.join(lines)


def deserialize_account(r: MemoryReader, offset: int, max_permitted_data_increase: int) -> Tuple[dict, int]:
    acc = {}
    acc['is_signer'] = r.u8(offset) != 0; offset += 1
    acc['is_writable'] = r.u8(offset) != 0; offset += 1
    acc['executable'] = r.u8(offset) != 0; offset += 1
    acc['original_data_len'] = r.u32(offset); offset += 4
    acc['key'] = r.pubkey(offset); offset += PUBKEY_SIZE
    acc['owner'] = r.pubkey(offset); offset += PUBKEY_SIZE
    acc['lamports'] = r.u64(offset); offset += 8
    acc['data_len'] = r.u64(offset); offset += 8
    acc['data_offset'] = offset
    acc['data'] = r.read(offset, acc['data_len']) if acc['data_len'] > 0 else b''
    offset += acc['data_len'] + max_permitted_data_increase
    offset = align(offset)
    acc['rent_epoch'] = r.u64(offset); offset += 8
    return acc, offset


def deserialize(r: MemoryReader, max_permitted_data_increase: int = MAX_PERMITTED_DATA_INCREASE):
    offset = 0
    num_accounts = r.u64(offset); offset += 8

    accounts = []
    for _ in range(num_accounts):
        acc_offset = offset
        dup = r.u8(offset); offset += 1
        if dup == NON_DUP_MARKER:
            acc, offset = deserialize_account(r, offset, max_permitted_data_increase)
            acc['offset'] = acc_offset
            acc['dup_marker'] = dup
            accounts.append(acc)
        else:
            offset += 7  # padding
            accounts.append({'dup_of': dup})

    ix_len = r.u64(offset); offset += 8
    ix_data_offset = offset
    ix_data = r.read(offset, ix_len); offset += ix_len
    program_id_offset = offset
    program_id = r.pubkey(offset); offset += PUBKEY_SIZE

    return {
        'program_id': program_id,
        'program_id_offset': program_id_offset,
        'accounts': accounts,
        'instruction_data': ix_data,
        'instruction_data_offset': ix_data_offset,
        'instruction_data_len': ix_len,
        'total_size': offset,
    }


def format_result(result, base: int) -> str:
    lines = []
    lines.append("=" * 60)
    lines.append(f"Accounts: {len(result['accounts'])}")
    lines.append("-" * 60)
    for i, acc in enumerate(result['accounts']):
        if 'dup_of' in acc:
            lines.append(f"  [{i}] DUPLICATE of #{acc['dup_of']}")
        else:
            lines.append(f"  [{i}] @ 0x{base + acc['offset']:x}")
            lines.append(f"      pubkey: {acc['key']}")
            lines.append(f"      owner: {acc['owner']}")
            lines.append(f"      lamports: {acc['lamports']} ({acc['lamports']/1e9:.9f} SOL)")
            lines.append(f"      rent_epoch: {acc['rent_epoch']}")
            lines.append(f"      dup_marker: 0x{acc['dup_marker']:02x}, signer: {acc['is_signer']}, writable: {acc['is_writable']}, executable: {acc['executable']}")
            lines.append(f"      original_data_len: {acc['original_data_len']}, data_len: {acc['data_len']}")
            data_addr = base + acc['data_offset']
            lines.append(f"      data @ 0x{data_addr:x}: x/{acc['data_len']}b 0x{data_addr:x}")
            if acc['data_len'] > 0:
                display_data = acc['data'][:128]
                lines.append(format_hex(display_data))
                if acc['data_len'] > 128:
                    lines.append(f"      ... ({acc['data_len'] - 128} more bytes)")
    lines.append("-" * 60)
    ix_addr = base + result['instruction_data_offset']
    ix_len = result['instruction_data_len']
    lines.append(f"Instruction data ({ix_len} bytes @ 0x{ix_addr:x}): x/{ix_len}b 0x{ix_addr:x}")
    if ix_len > 0:
        display_ix = result['instruction_data'][:128]
        lines.append(format_hex(display_ix))
        if ix_len > 128:
            lines.append(f"      ... ({ix_len - 128} more bytes)")
    lines.append("-" * 60)
    lines.append(f"Program ID: {result['program_id']} (address: 0x{base + result['program_id_offset']:x})")
    lines.append("-" * 60)
    lines.append(f"Total input buffer size: {result['total_size']} bytes (0x{result['total_size']:x})")
    lines.append("=" * 60)
    return "\n".join(lines)


def solana_input_deserialize_abiv1_command(debugger, command, result, internal_dict):
    args = command.strip().split()
    base = int(args[0], 0) if len(args) > 0 else INPUT_BASE_ADDRESS
    max_permitted_data_increase = int(args[1], 0) if len(args) > 1 else MAX_PERMITTED_DATA_INCREASE

    print(f"Deserializing at 0x{base:x} (max_permitted_data_increase={max_permitted_data_increase})...")
    try:
        process = debugger.GetSelectedTarget().GetProcess()
        r = MemoryReader(process, base)
        res = deserialize(r, max_permitted_data_increase)
        output = format_result(res, base)
        print(output)
    except Exception as e:
        print(f"Error: {e}")
        import traceback; traceback.print_exc()


def __lldb_init_module(debugger, internal_dict):
    debugger.HandleCommand(
        'command script add -f solana_input_deserialize_abiv1.solana_input_deserialize_abiv1_command solana_input_deserialize_abiv1'
    )
    print(f"Loaded. Use: solana_input_deserialize_abiv1 [addr] [max_permitted_data_increase]")


# =============================================================================
# Base58 encoding
# From: https://github.com/keis/base58 (v2.1.1)
# License: MIT - Copyright (c) 2013 David Keijser
# SPDX-License-Identifier: MIT
# =============================================================================
B58_ALPHABET = b'123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz'


def b58encode(v: bytes) -> bytes:
    origlen = len(v)
    v = v.lstrip(b'\0')
    newlen = len(v)

    acc = int.from_bytes(v, byteorder='big')

    result = b''
    while acc:
        acc, idx = divmod(acc, 58)
        result = B58_ALPHABET[idx:idx+1] + result

    return B58_ALPHABET[0:1] * (origlen - newlen) + result
