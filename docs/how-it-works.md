# How wish works

The front page shows what wish does. This page walks through `wish.cpp` and lists the exact rules it follows.

## Reading a line

`main()` reads one line at a time with `getline()`. In interactive mode it prints `wish> ` first. In batch mode it reads from the file and prints no prompt. It stops at the end of the input or at `exit`.

`tokenize()` splits the line on whitespace. It also makes `>` and `&` tokens of their own, so `ls>out` and `ls&pwd` work without spaces.

## Running the commands on a line

`main()` cuts the tokens at each `&` and hands each part to `run_command()`. That function starts a program and returns without waiting, so the shell starts the programs on a line one after another and only then waits for them. It waits for each child with `waitpid()`, and tries again if the call fails with `EINTR`.

`run_command()` does the rest:

1. It checks the redirect first. A `>` needs a command before it and exactly one file name after it.
2. Built-ins run inside the shell itself.
3. For any other command it tries each directory in the search path with `access()` and `X_OK`, and takes the first match.
4. It calls `fork()`. With a redirect, the child opens the file, creating or truncating it, and points standard output and standard error at it with `dup2()`.
5. The child calls `execv()`. If that returns, the child prints the error and exits with `_exit(1)`, so it never goes on as a second copy of the shell.

## Built-ins

- `exit` takes no arguments and ends the shell.
- `cd` takes exactly one argument and calls `chdir()`.
- `path` replaces the search path with its arguments. With no arguments the path is empty, and only the built-ins still work.

A built-in with a redirect is an error.

## Errors

`error()` writes `An error has occurred` to standard error with `write()`. It loops on short writes and tries again if the call is interrupted.

After an error in a command, the shell goes on to the next one. It exits with status 1 in two cases: it was started with more than one argument, or it couldn't open the batch file.

## Tests

`run_tests.sh` downloads the OSTEP projects repository at commit `76cff3f` into `.ostep/`, copies `wish` into its `processes-shell/` folder and runs the project's `test-wish.sh`. It only downloads the repository the first time. Extra arguments go to `test-wish.sh`, for example `-c` to keep going after a failure. GitHub Actions runs `make` and `make test` on every push and pull request.
