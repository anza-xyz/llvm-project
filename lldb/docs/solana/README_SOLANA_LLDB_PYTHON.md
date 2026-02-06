# Solana LLDB Python Issues

This document describes Python module path issues when using `solana-lldb` across different operating systems.

## Ubuntu 22.04

Ubuntu 22.04 ships with Python 3.10, which is the version Solana platform-tools expects.

### The Problem

The bundled `lldb` cannot find the `lldb` module because it looks in the wrong `dist-packages` directory:

```bash
$ ~/.cache/solana/v1.53/platform-tools/llvm/bin/lldb -P
Traceback (most recent call last):
  File "<string>", line 1, in <module>
ModuleNotFoundError: No module named 'lldb.embedded_interpreter'
/home/myuser/.cache/solana/v1.53/platform-tools/llvm/local/lib/python3.10/dist-packages
```

The directory it's looking for doesn't exist:

```bash
$ ls /home/myuser/.cache/solana/v1.53/platform-tools/llvm/local/lib/python3.10/dist-packages
ls: cannot access '...': No such file or directory
```

### The Solution

Export the correct `PYTHONPATH` before running `solana-lldb`:

```bash
export PYTHONPATH=/home/myuser/.cache/solana/v1.53/platform-tools/llvm/lib/python3.10/dist-packages:$PYTHONPATH
~/.cache/solana/v1.53/platform-tools/llvm/bin/solana-lldb
```

After setting `PYTHONPATH`, `solana-lldb` works correctly:

```
(lldb) command script import "/home/myuser/.cache/solana/v1.53/platform-tools/llvm/bin/lldb_lookup.py"
(lldb) command script import "/home/myuser/.cache/solana/v1.53/platform-tools/llvm/bin/solana_lookup.py"
(lldb) command source -s 0 '/home/myuser/.cache/solana/v1.53/platform-tools/llvm/bin/lldb_commands'
Executing commands in '/home/myuser/.cache/solana/v1.53/platform-tools/llvm/bin/lldb_commands'.
(lldb) type synthetic add -l lldb_lookup.synthetic_lookup -x ".*" --category Rust
(lldb) type summary add -F lldb_lookup.summary_lookup  -e -x -h "^(alloc::([a-z_]+::)+)String$" --category Rust
(lldb) type summary add -F lldb_lookup.summary_lookup  -e -x -h "^&(mut )?str$" --category Rust
(lldb) type summary add -F lldb_lookup.summary_lookup  -e -x -h "^&(mut )?\\[.+\\]$" --category Rust
(lldb) type summary add -F lldb_lookup.summary_lookup  -e -x -h "^(std::ffi::([a-z_]+::)+)OsString$" --category Rust
(lldb) type summary add -F lldb_lookup.summary_lookup  -e -x -h "^(alloc::([a-z_]+::)+)Vec<.+>$" --category Rust
(lldb) type summary add -F lldb_lookup.summary_lookup  -e -x -h "^(alloc::([a-z_]+::)+)VecDeque<.+>$" --category Rust
(lldb) type summary add -F lldb_lookup.summary_lookup  -e -x -h "^(alloc::([a-z_]+::)+)BTreeSet<.+>$" --category Rust
(lldb) type summary add -F lldb_lookup.summary_lookup  -e -x -h "^(alloc::([a-z_]+::)+)BTreeMap<.+>$" --category Rust
(lldb) type summary add -F lldb_lookup.summary_lookup  -e -x -h "^(std::collections::([a-z_]+::)+)HashMap<.+>$" --category Rust
(lldb) type summary add -F lldb_lookup.summary_lookup  -e -x -h "^(std::collections::([a-z_]+::)+)HashSet<.+>$" --category Rust
(lldb) type summary add -F lldb_lookup.summary_lookup  -e -x -h "^(alloc::([a-z_]+::)+)Rc<.+>$" --category Rust
(lldb) type summary add -F lldb_lookup.summary_lookup  -e -x -h "^(alloc::([a-z_]+::)+)Arc<.+>$" --category Rust
(lldb) type summary add -F lldb_lookup.summary_lookup  -e -x -h "^(core::([a-z_]+::)+)Cell<.+>$" --category Rust
(lldb) type summary add -F lldb_lookup.summary_lookup  -e -x -h "^(core::([a-z_]+::)+)Ref<.+>$" --category Rust
(lldb) type summary add -F lldb_lookup.summary_lookup  -e -x -h "^(core::([a-z_]+::)+)RefMut<.+>$" --category Rust
(lldb) type summary add -F lldb_lookup.summary_lookup  -e -x -h "^(core::([a-z_]+::)+)RefCell<.+>$" --category Rust
(lldb) type category enable Rust
(lldb) command source -s 0 '/home/myuser/.cache/solana/v1.53/platform-tools/llvm/bin/solana_commands'
Executing commands in '/home/myuser/.cache/solana/v1.53/platform-tools/llvm/bin/solana_commands'.
(lldb) type summary add -F solana_lookup.summary_lookup solana_program::account_info::AccountInfo --category Solana
(lldb) type summary add -F solana_lookup.summary_lookup solana_program::pubkey::Pubkey --category Solana
(lldb) type category enable Solana
(lldb)
```

## Ubuntu 24.04

### The Problem

Ubuntu 24.04 ships with Python 3.12, but the Solana platform-tools depend on Python 3.10.

### The Solution

Install Python 3.10 from the deadsnakes PPA:

```bash
sudo apt update
sudo apt install software-properties-common
sudo add-apt-repository ppa:deadsnakes/ppa
sudo apt update
sudo apt install python3.10 python3.10-venv python3.10-dev
```

Then set the `PYTHONPATH` (same as Ubuntu 22.04):

```bash
export PYTHONPATH=~/.cache/solana/v1.53/platform-tools/llvm/lib/python3.10/dist-packages:$PYTHONPATH
~/.cache/solana/v1.53/platform-tools/llvm/bin/solana-lldb
```

After this, `solana-lldb` works correctly and loads Rust/Solana type summaries (same output as Ubuntu 22.04).

### Example: Importing and Using a Custom Script

Once `solana-lldb` is working, you can import custom Python scripts:

```
(lldb) command script import ./solana_input_deserialize_abiv1.py
Loaded. Use: solana_input_deserialize_abiv1 [addr] [max_permitted_data_increase]
(lldb) gdb-remote 10.156.130.1:9090
Process 1 stopped
* thread #1, stop reason = signal SIGTRAP
    frame #0: 0x586f010000000000
(lldb) solana_input_deserialize_abiv1
Deserializing at 0x400000000 (max_permitted_data_increase=10240)...
....
```

## macOS

On macOS, LLDB works out of the box because Solana platform-tools is tied to python3.14 (unlike the builds for Linux, which use python3.10 since platform-tools is built in an Ubuntu 22.04 environment).

Ensure python3.14 is installed via Homebrew:

```bash
$ brew list | grep python@3.14
python@3.14
```

```bash
$ ~/.cache/solana/v1.53/platform-tools/llvm/bin/solana-lldb -P
/Users/myuser/.cache/solana/v1.53/platform-tools/llvm/lib/python3.14/site-packages
```

macOS "just works" because the path to the module is correct.
