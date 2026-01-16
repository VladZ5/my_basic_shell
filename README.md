# Custom Unix Shell in C

A minimal Unix shell implementation built from scratch in C, featuring command execution, built-in commands, I/O redirection, and pipes.

## Features

- **Command Execution**: Runs external programs using fork/exec
- **Built-in Commands**: 
  - `cd` - Change directory (supports `~` for home)
  - `exit` - Exit the shell
- **I/O Redirection**:
  - `>` - Redirect output (overwrite)
  - `>>` - Redirect output (append)
  - `<` - Redirect input
- **Pipes**: Chain commands with `|`
- **Dynamic Memory**: Grows argument array as needed

## Build & Run
```bash
gcc main.c builtins.c -o myshell
./myshell
```

## Examples
```bash
dacoolestshell> ls -la
dacoolestshell> cat file.txt | grep "error" | wc -l
dacoolestshell> ls > output.txt
dacoolestshell> cat < input.txt > output.txt
dacoolestshell> cd ~/Documents
```

## Learning Goals

Built to understand:
- Process management (fork, exec, wait)
- File descriptors and I/O redirection
- Inter-process communication via pipes
- Memory management in C
