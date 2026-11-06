# yacsh

Yet Another POSIX-compliant Shell written in C.

Wannabe [ash](https://en.wikipedia.org/wiki/Almquist_shell?useskin=vector)

## Supported Features

- Child process: `fork()` + `exec()`
- Redirection: Input, and Output
- Scripts
- CLI Editing
- Shell Variables
- Pipes
- Environment Variables

## Development Setup

Compile and execute the shell.

```sh
clang -std=c99 -fsanitize=address -Wall -Werror -o yacsh main.c
```

## Development Guide

- Use C99 standard for writing code.
- Use a single file to avoid "include hell". If unavoidable, use a single translation unit or Unity Build.
- Use an address sanitizer for local development (`-fsanitize=address`).
