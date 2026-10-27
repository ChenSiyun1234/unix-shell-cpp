# wish

A small Unix shell I wrote in C++.

```
$ ./wish
wish> echo hello > out.txt & ls missing > err.txt
wish> cat out.txt err.txt
hello
ls: cannot access 'missing': No such file or directory
wish> path
wish> ls
An error has occurred
wish> exit
```

The first line I typed runs `echo` and `ls` at the same time. The `ls` error lands in `err.txt`, because `>` sends both standard output and standard error to the file. `path` with nothing after it empties the search path, so after that `ls` can't be found.

## What I did

I wrote wish as one C++ file of about 175 lines. It follows the processes-shell project from *Operating Systems: Three Easy Pieces*, which is also where the name comes from. That project spells out how the shell should behave and comes with a test suite.

The shell reads commands at the `wish>` prompt, or from a file if you pass one. To run a program, it checks each directory in its search path with `access()` and takes the first match. The path starts as `/bin`. Then it calls `fork()`, and the child calls `execv()`. With `cmd > file`, the child points standard output and standard error at the file with `dup2()` before it calls `execv()`.

Programs joined by `&` run at the same time. The shell starts them one after another without waiting, then waits for each one with `waitpid()` before it reads the next line.

The built-ins are `exit`, `cd` and `path`. If a command can't run, wish prints the one error message the project asks for and moves on.

## Design choices

Built-ins have to run inside the shell itself. A child process can't change the shell's working directory or its search path, and `exit` in a child would only end the child.

I made `>` and `&` tokens of their own in the tokenizer, so `ls>out` and `ls&pwd` work without spaces.

If `execv()` fails, the child prints the error and exits right away. Otherwise it would carry on as a second copy of the shell.

## Build and test

```
make
make test
```

It builds with `g++` on Linux. `make test` downloads the processes-shell tests from the OSTEP projects repository at a fixed commit and runs all 22 of them. GitHub Actions runs the same two steps on every push. A walk through the code and the exact error rules are in [docs/how-it-works.md](docs/how-it-works.md).

## Credits

The shell's behavior and the test suite come from the processes-shell project in *Operating Systems: Three Easy Pieces* by Remzi and Andrea Arpaci-Dusseau: https://github.com/remzi-arpacidusseau/ostep-projects
