# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Write the lit statuses and underlying Dexter scores to a CI job summary."""

import csv
import json
from pathlib import Path
import sys

root = Path(sys.argv[1])
report = root / "lit.json"
if not report.exists():
    print("SBPF Dexter did not produce a lit report. Inspect the failed job step.")
    sys.exit(0)

print("## SBPF debug-info quality\n")
print("Rust debugging through LLDB and the SBPF interpreter.\n")
print("| Test | Status | Dexter score |")
print("| --- | --- | --- |")
tests = json.loads(report.read_text())["tests"]
for test in sorted(tests, key=lambda test: test["name"]):
    name = test["name"].split(" :: ")[-1]
    summaries = sorted(
        (root / Path(name).stem).glob("dexter-*/summary.csv"),
        key=lambda path: path.stat().st_mtime,
    )
    score = "unavailable"
    if summaries:
        with summaries[-1].open(newline="") as stream:
            rows = list(csv.DictReader(stream))
        if len(rows) == 1 and not rows[0].get("Error"):
            score = rows[0].get("Score", "unavailable")
    print("| {} | {} | {} |".format(name, test["code"], score))
print(
    "\nXFAIL records a known defect; XPASS requires removing its expectation. "
    "Infrastructure errors remain failures. Full observations are in the artifact."
)
