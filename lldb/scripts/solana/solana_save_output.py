"""
LLDB Command Output Saver

Captures the output of any LLDB command and saves it to a file.

Usage in LLDB:
    (lldb) command script import /path/to/solana_save_output.py
    (lldb) solana_save_output /tmp/out.txt memory read 0x400000000 -c 100
    (lldb) solana_save_output /tmp/regs.txt register read
    (lldb) solana_save_output /tmp/bt.txt bt
"""


def solana_save_output_command(debugger, command, result, internal_dict):
    import lldb
    import sys
    import io

    raw = command.strip()
    if raw.startswith('"') or raw.startswith("'"):
        quote = raw[0]
        end = raw.find(quote, 1)
        if end == -1:
            print("Unterminated quote in filename.")
            return
        filename = raw[1:end]
        lldb_command = raw[end+1:].strip()
    else:
        parts = raw.split(None, 1)
        filename = parts[0] if parts else ""
        lldb_command = parts[1] if len(parts) > 1 else ""

    if not filename or not lldb_command:
        print("Usage: solana_save_output <filename> <lldb command>")
        print('Example: solana_save_output "/tmp/my out.txt" memory read 0x400000000')
        return

    # Capture stdout (for Python print() calls) and LLDB output
    captured_stdout = io.StringIO()
    old_stdout = sys.stdout
    sys.stdout = captured_stdout

    try:
        ret = lldb.SBCommandReturnObject()
        interpreter = debugger.GetCommandInterpreter()
        interpreter.HandleCommand(lldb_command, ret)
        lldb_output = ret.GetOutput() or ""
        lldb_error = ret.GetError() or ""
    finally:
        sys.stdout = old_stdout

    stdout_output = captured_stdout.getvalue()

    # Combine all outputs
    all_output = stdout_output + lldb_output + lldb_error

    if not ret.Succeeded():
        print("LLDB command failed.")
        if lldb_error:
            print(lldb_error, end="")
        return

    if not all_output:
        print("No output captured.")
        return

    # Print to console
    print(all_output, end="")

    # Save to file
    try:
        with open(filename, 'w') as f:
            f.write(all_output)
    except OSError as e:
        print(f"Failed to write to {filename}: {e}")
        return

    print(f"\n[Saved to: {filename}]")


def __lldb_init_module(debugger, internal_dict):
    debugger.HandleCommand(
        'command script add -f solana_save_output.solana_save_output_command solana_save_output'
    )
    print("Loaded. Use: solana_save_output <filename> <lldb command>")
