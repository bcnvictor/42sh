# 42sh

A POSIX-compliant shell written in C, built as part of the EPITA system programming curriculum.

---

## Features

- **Interactive mode** — reads commands from stdin with a prompt
- **Script mode** — executes a shell script file
- **`-c` flag** — executes a command string directly
- **Builtins**: `echo`, `cd`, `exit`, `export`, `unset`, `alias`, `unalias`, `.` (dot), `true`, `false`, `break`, `continue`
- **Control flow**: `if/elif/else/fi`, `while`, `until`, `for/in`, `case/esac`
- **Pipelines** (`|`), logical operators (`&&`, `||`), negation (`!`)
- **Redirections**: `<`, `>`, `>>`, `<&`, `>&`, `>|`, `<>`
- **Variable expansion**: `$VAR`, `${VAR}`, `$1`…`$n`, `$?`, `$$`, `$#`, `$*`, `$@`, `$RANDOM`, `$UID`
- **Command substitution**: `$(...)` and `` `...` ``
- **Quote handling**: single quotes, double quotes, backslash escaping
- **Subshells**: `(compound list)`
- **Functions**: definition and call
- **Aliases**

---

## Architecture

```
src/
├── 42sh.c              # Entry point: input loop, AST build/run cycle
├── lexer/              # Tokenizer (lexer.c, print.c)
├── parser/             # Recursive descent parser → AST nodes (parser.c)
├── ast/                # AST node types, evaluation dispatcher, memory cleanup
│   ├── ast.c           # Node representation
│   ├── evaluation.c    # run_ast() dispatch + all execution logic
│   └── free_node.c     # Recursive AST freeing
├── builtins/           # Built-in command implementations
│   ├── builtins_s1_2.c # echo, true, false, args dispatch
│   └── builtins_s3.c   # cd, export, exit, unset, dot, break, continue, alias
├── io_backend/         # Input handling, word splitting, quote/escape parsing
│   └── handle_input.c
└── utils/
    ├── context.c       # Global shell context: variables, functions, aliases, args
    ├── expansion.c     # Variable and command substitution expansion engine
    ├── expand_variables.c  # Command substitution execution
    └── myutils.c       # Token classification helpers
```

**Execution pipeline:**

```
input → lexer (tokens) → parser (AST) → run_ast() → builtins / execvp
```

The shell is entirely single-process except for pipelines, subshells, and external commands, which each `fork()`. The global `context` struct holds all shell state (variables, functions, aliases, return code, loop depth).

---

## Build

Requires `gcc`, `make`, and the GNU autotools (`autoconf`, `automake`).

```bash
autoreconf -fi
./configure
make
```

The binary is produced at `src/42sh`.

To clean build artifacts:

```bash
make clean
```

---

## Usage

```bash
# Interactive mode
./src/42sh

# Execute a script
./src/42sh script.sh

# Execute a command string
./src/42sh -c "echo hello world"
```

---

## Tests

```bash
cd tests
bash tests.sh
```

Test scripts are in `tests/shell_scripts/`.
