# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
import csv
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import MagicMock
from types import SimpleNamespace

import lit.Test
from sbpf_format import classify, run_logged
from dex.debugger.lldb.LLDB import LLDB
from dex.utils.Exceptions import DebuggerException


class ClassificationTests(unittest.TestCase):
    def classify(self, score, error="", status=2):
        with tempfile.TemporaryDirectory() as directory:
            summary = Path(directory) / "summary.csv"
            with summary.open("w", newline="") as stream:
                writer = csv.writer(stream)
                writer.writerow(["Test Case", "Score", "Error"])
                writer.writerow(["fixture", score, error])
            return classify(status, summary)[0]

    def test_semantic_failure_can_be_xfailed(self):
        self.assertEqual(self.classify(0.5), lit.Test.FAIL)

    def test_debugger_error_cannot_be_xfailed(self):
        self.assertEqual(self.classify(0, "connection failed"), lit.Test.UNRESOLVED)

    def test_success(self):
        self.assertEqual(self.classify(1, status=0), lit.Test.PASS)

    def test_invalid_score_or_exit_is_not_a_debug_info_failure(self):
        for score, status in [("nan", 2), ("inf", 2), (1, 2), (0, 0), (0, -9)]:
            with self.subTest(score=score, status=status):
                self.assertEqual(
                    self.classify(score, status=status), lit.Test.UNRESOLVED
                )

    def test_timeout_is_not_swallowed(self):
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaises(subprocess.TimeoutExpired):
                run_logged(
                    [sys.executable, "-c", "import time; time.sleep(30)"],
                    Path(directory) / "log",
                    timeout=0.1,
                )


class RemoteLaunchTests(unittest.TestCase):
    def setUp(self):
        self.adapter = object.__new__(LLDB)
        self.adapter.context = SimpleNamespace(
            options=SimpleNamespace(
                lldb_remote_url="connect://127.0.0.1:1234", target_run_args=None
            )
        )
        self.adapter._interface = MagicMock()
        self.adapter._interface.eStopReasonBreakpoint = 3
        self.error = self.adapter._interface.SBError.return_value
        self.error.Fail.return_value = False
        self.adapter._debugger = MagicMock()
        self.adapter._target = MagicMock()
        self.adapter._target.executable.fullpath = __file__
        breakpoint = MagicMock()
        breakpoint.GetNumLocations.return_value = 1
        self.adapter._target.breakpoint_iter.return_value = [breakpoint]
        self.process = MagicMock()
        self.process.GetNumThreads.return_value = 1
        self.thread = self.process.GetThreadAtIndex.return_value
        self.thread.GetStopReason.return_value = 3
        self.process.__iter__.side_effect = lambda: iter([self.thread])
        self.process.Continue.return_value.Fail.return_value = False
        self.adapter._target.ConnectRemote.return_value = self.process
        self.adapter._target.Launch.return_value = self.process

    def test_remote_initial_signal_continues_to_breakpoint(self):
        self.thread.GetStopReason.side_effect = [5, 3]
        self.adapter.launch([])
        self.adapter._target.Launch.assert_not_called()
        self.process.Continue.assert_called_once()
        self.adapter._target.ConnectRemote.assert_called_once_with(
            self.adapter._debugger.GetListener(),
            "connect://127.0.0.1:1234",
            "gdb-remote",
            self.error,
        )

    def test_remote_initial_breakpoint_is_not_skipped(self):
        self.adapter.launch([])
        self.process.Continue.assert_not_called()

    def test_local_launch_is_preserved(self):
        self.adapter.context.options.lldb_remote_url = None
        self.adapter.launch(["argument"])
        self.adapter._target.ConnectRemote.assert_not_called()
        self.adapter._target.GetLaunchInfo().SetArguments.assert_called_once_with(
            ["argument"], True
        )
        self.adapter._target.Launch.assert_called_once()

    def test_connection_error_is_reported(self):
        self.error.Fail.return_value = True
        self.error.GetCString.return_value = "connection refused"
        with self.assertRaisesRegex(DebuggerException, "connection refused"):
            self.adapter.launch([])

    def test_remote_launch_arguments_are_rejected(self):
        with self.assertRaisesRegex(DebuggerException, "launch arguments"):
            self.adapter.launch(["argument"])
        self.adapter._target.ConnectRemote.assert_not_called()

    def test_signal_after_continue_is_not_a_successful_launch(self):
        self.thread.GetStopReason.return_value = 5
        with self.assertRaisesRegex(DebuggerException, "test breakpoint"):
            self.adapter.launch([])
