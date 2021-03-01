# PtyProcess package for U++

This package provides a unified and easy-to use interface for POSIX pty, Window 10
pseudoconsole API or [winpty](https://github.com/rprichard/winpty) on Windows XP/7/8/10.

## Notes:

- See the terminal examples for PtyProcess reference examples.
- WinPty backend only requires `winpty-agent` and the `winpty.dll` and its import library `winpty.lib`. (It does not require `Cygwin` or `MSYS2`)
- WinPty is not provided with the package.
