# wish: a small Unix shell in C++

[![test](https://github.com/ChenSiyun1234/unix-shell-cpp/actions/workflows/test.yml/badge.svg)](https://github.com/ChenSiyun1234/unix-shell-cpp/actions/workflows/test.yml)

wish is a command-line shell for Linux, written in C++. It reads commands from the keyboard or from a batch file and runs each program in its own child process.

## Features

- Interactive mode prints `wish> ` and reads one line at a time. Batch mode runs every line of a file, for example `./wish commands.txt`, and prints no prompt.
- Programs are looked up in the shell's search path, which starts as `/bin`. The first directory that holds an executable with that name wins.
- `cmd > file` sends both the output and the error output of `cmd` to `file`.
- `cmd1 & cmd2 & cmd3` starts each command without waiting for the one before it. The shell then waits for all of them before it reads the next line.
- Three commands are built in: `exit`, `cd <dir>` and `path [dir ...]`. `path` replaces the search path. With no arguments it empties the path, so only the built-ins still work.
- Every error prints `An error has occurred` on standard error, and the shell keeps reading. A batch file that cannot be opened, or more than one argument, makes it exit with status 1.

## Build and test

```
make
make test
```

`make test` downloads the processes-shell tests from the OSTEP projects repository at a pinned commit and runs all 22 of them. GitHub Actions runs the same steps on every push.

## How it works

- `tokenize()` splits a line on whitespace. It also makes `>` and `&` their own tokens, so `ls>out` and `ls&pwd` work without spaces.
- `main()` cuts the token list at each `&` and hands each part to `run_command()`. It collects the child process IDs and waits for all of them with `waitpid()`, retrying when a signal interrupts the call.
- `run_command()` checks the redirection syntax first. A `>` needs a command before it and exactly one file name after it. Built-ins run inside the shell. Any other program is found with `access()`, and then the shell calls `fork()`.
- With a redirection, the child opens the output file and points standard output and standard error at it with `dup2()`. Then it calls `execv()`. If `execv()` returns, the child reports the error and exits.
- `error()` writes the message with `write()` and retries when a write is cut short or interrupted.

## Credits

The shell's behavior and the test suite come from the processes-shell project in *Operating Systems: Three Easy Pieces* by Remzi and Andrea Arpaci-Dusseau: https://github.com/remzi-arpacidusseau/ostep-projects
