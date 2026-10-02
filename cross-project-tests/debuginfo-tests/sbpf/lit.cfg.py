# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from sbpf_format import SBPFDexterTest

config.name = "SBPF Dexter"
config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.path.abspath(
    lit_config.params.get("sbpf_output", "sbpf-dexter-results")
)
config.excludes = ["Inputs", "unit", "__pycache__"]
config.test_format = SBPFDexterTest()
config.sbpf_tools = {}
for name in ("lld", "lldb", "rustc", "sbpf_cli", "python"):
    value = lit_config.params.get(name)
    if not value or not os.path.isfile(value):
        lit_config.fatal("supply --param {}=/absolute/path/to/tool".format(name))
    config.sbpf_tools[name] = os.path.abspath(value)
