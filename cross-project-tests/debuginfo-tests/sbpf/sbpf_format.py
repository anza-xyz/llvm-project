# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Run real Dexter observations; only scored semantic failures may be XFAIL.

The dedicated format keeps compilation, launch, timeout, and debugger errors
UNRESOLVED, which lit does not turn into XFAIL. No shell result is masked.
"""

import csv
import json
import math
import os
from pathlib import Path
import signal
import socket
import subprocess
import tempfile
import time

import lit.formats
import lit.Test


def classify(returncode, summary):
    with open(summary, newline="") as stream:
        rows = list(csv.DictReader(stream))
    if len(rows) != 1 or rows[0]["Error"]:
        return lit.Test.UNRESOLVED, "Dexter did not produce one successful observation"
    score = float(rows[0]["Score"])
    if not math.isfinite(score) or not 0 <= score <= 1 or returncode not in (0, 2):
        return lit.Test.UNRESOLVED, "invalid score or Dexter exit status"
    if (score == 1.0) != (returncode == 0):
        return lit.Test.UNRESOLVED, "inconsistent score and Dexter exit status"
    return (lit.Test.PASS if score == 1.0 else lit.Test.FAIL), "score={:.4f}".format(
        score
    )


def stop_process(process):
    # Dexter starts its own Python debugger child. Kill the entire private
    # process group on timeout so no child or occupied port survives the test.
    if process.poll() is None:
        os.killpg(process.pid, signal.SIGTERM)
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGKILL)
            process.wait()


def run_logged(command, logfile, timeout=60, env=None):
    with open(logfile, "w") as stream:
        stream.write("command: " + repr(command) + "\n")
        stream.flush()
        process = subprocess.Popen(
            command,
            stdout=stream,
            stderr=subprocess.STDOUT,
            env=env,
            start_new_session=True,
        )
        try:
            return process.wait(timeout=timeout)
        finally:
            stop_process(process)


class SBPFDexterTest(lit.formats.TestFormat):
    def getTestsInDirectory(self, testSuite, path_in_suite, litConfig, localConfig):
        directory = testSuite.getSourcePath(path_in_suite)
        for name in sorted(os.listdir(directory)):
            if name.endswith(".test"):
                yield lit.Test.Test(testSuite, path_in_suite + (name,), localConfig)

    def execute(self, test, litConfig):
        root = Path(test.config.test_source_root)
        output = Path(test.getExecPath()).with_suffix("")
        output.mkdir(parents=True, exist_ok=True)
        try:
            case = json.loads(Path(test.getSourcePath()).read_text())
            if case.get("xfail"):
                test.xfails = ["*"]
            code, detail = self.run_case(case, root, output, test.config.sbpf_tools)
        except (OSError, ValueError, KeyError, subprocess.TimeoutExpired) as error:
            code, detail = lit.Test.UNRESOLVED, str(error)
        for log in ("build.log", "link.log", "vm.log", "dexter.log"):
            path = output / log
            if path.exists():
                detail += "\n{}:\n{}".format(log, path.read_text()[-16000:])
        return lit.Test.Result(code, detail)

    def run_case(self, case, root, output, tools):
        source = root / "Inputs" / case["source"]
        obj, elf = output / "fixture.o", output / "fixture.so"
        command = [
            tools["rustc"],
            str(source),
            "--crate-type=lib",
            "--target=sbpf-solana-solana",
            "--emit=obj",
            "-g",
            "-C",
            "panic=abort",
            "-C",
            "opt-level=2",
            "-o",
            str(obj),
        ]
        if run_logged(command, output / "build.log"):
            return lit.Test.UNRESOLVED, "fixture compilation failed"
        command = [
            tools["lld"],
            "-z",
            "notext",
            "-shared",
            "--Bdynamic",
            "--entry",
            "entrypoint",
            "--script",
            str(root / "Inputs" / "sbpf.ld"),
            str(obj),
            "-o",
            str(elf),
        ]
        if run_logged(command, output / "link.log"):
            return lit.Test.UNRESOLVED, "fixture link failed"

        # Reserve a fresh localhost port, then wait for the stub's readiness
        # log. A TCP probe would consume its only debugger connection.
        with socket.socket() as sock:
            sock.bind(("127.0.0.1", 0))
            port = sock.getsockname()[1]
        env = dict(os.environ, VM_DEBUG_PORT=str(port))
        with open(output / "vm.log", "w") as log:
            vm = subprocess.Popen(
                [
                    tools["sbpf_cli"],
                    "--elf",
                    str(elf),
                    "--use",
                    "interpreter",
                    "--input",
                    "8",
                    "--lim",
                    "10000",
                ],
                env=env,
                stdout=log,
                stderr=subprocess.STDOUT,
                start_new_session=True,
            )
            try:
                deadline = time.monotonic() + 15
                while (
                    "Waiting for a Debugger connection"
                    not in (output / "vm.log").read_text()
                ):
                    if vm.poll() is not None or time.monotonic() > deadline:
                        return lit.Test.UNRESOLVED, "SBPF stub failed to become ready"
                    time.sleep(0.05)
                # Use a different directory on each invocation: a stale report
                # must never turn a failed run into an expected failure.
                results = Path(tempfile.mkdtemp(prefix="dexter-", dir=output))
                command = [
                    tools["python"],
                    str(root.parent / "dexter" / "dexter.py"),
                    "test",
                    "--debugger",
                    "lldb",
                    "--lldb-executable",
                    tools["lldb"],
                    "--lldb-remote-url",
                    "connect://127.0.0.1:" + str(port),
                    "--arch",
                    "sbpf",
                    "--binary",
                    str(elf),
                    "--fail-lt",
                    "1.0",
                    "--max-steps",
                    "100",
                    "--results-directory",
                    str(results),
                    "--",
                    str(root / "Inputs" / case["expectations"]),
                ]
                returncode = run_logged(command, output / "dexter.log", timeout=45)
                code, detail = classify(returncode, results / "summary.csv")
                # Successful observation must include a complete guest run.
                vm.wait(timeout=5)
                expected_result = case.get("return_value", 22)
                if (
                    vm.returncode
                    or "Result: Ok({})".format(expected_result)
                    not in (output / "vm.log").read_text().splitlines()
                ):
                    return (
                        lit.Test.UNRESOLVED,
                        "guest did not complete with expected value {}".format(
                            expected_result
                        ),
                    )
                return code, detail + "\n" + case.get("xfail", "")
            finally:
                stop_process(vm)
