# st3sh2

> A POSIX-compliant shell and multithreaded pseudo-terminal — built from scratch. \\
None of the code in this project was generated via AI.
However, most of this readme has been generated via AI.
---

## Overview

This repo contains two closely related projects:

**st3sh2** is the second iteration of `st3sh` — a handcrafted POSIX-compliant shell written in C. It supports the full command execution model you'd expect from a real shell: pipelines, redirections, command lists, quoting, builtins, and proper signal handling.

**pty** is the pseudo-terminal layer underneath it all — a multithreaded PTY implementation that manages master/slave terminal pairs, I/O forwarding, and process lifecycle.

Together they form a complete, low-level terminal stack.

---

# Demo
https://github.com/user-attachments/assets/5140eba4-ec2c-4085-8c65-8b8d5e429dc4

## st3sh2 — The Shell

### Command Parsing & Execution

st3sh2 features a hand-written lexer and parser that tokenizes input into a structured command tree before execution. Commands are parsed into distinct stages — tokenization, grouping, operator resolution — so the executor always works with a clean, validated representation of what the user typed.

Simple commands, compound commands, and operator-separated lists are all handled uniformly through the same execution pipeline.

### Separators & Command Lists

st3sh2 supports the full set of POSIX command list operators:

| Operator | Behavior |
|----------|----------|
| `;`      | Run sequentially; always continues |
| `&&`     | Run next command only if previous succeeded (`exit 0`) |
| `\|\|`     | Run next command only if previous failed (non-zero exit) |

These can be chained arbitrarily — `cmd1 && cmd2 \|\| cmd3 ; cmd4` is parsed and executed exactly as POSIX specifies.

### Pipelines

Pipelines connect the stdout of one command directly to the stdin of the next via kernel pipes. st3sh2 correctly:

- Forks a child process per pipeline stage
- Wires up `pipe(2)` file descriptors across the chain
- Closes unused ends in both parent and child
- Waits on all pipeline children and propagates the **exit status of the last command** in the pipeline

Multi-stage pipelines (`cmd1 | cmd2 | cmd3 | ...`) work to arbitrary depth.

### Redirections

st3sh2 implements the full POSIX redirection model:

| Syntax | Description |
|--------|-------------|
| `> file` | Redirect stdout to file (truncate) |
| `>> file` | Redirect stdout to file (append) |
| `< file` | Redirect stdin from file |
| `2> file` | Redirect stderr to file |
| `2>&1` | Redirect stderr to stdout |
| `>&2` | Redirect stdout to stderr |
| `n> file` | Redirect arbitrary file descriptor `n` |

Redirections are applied after `fork(2)` and before `exec(2)`, following the POSIX-specified order of operations. They compose correctly with pipelines.

### Built-in Commands

Certain commands must run inside the shell process itself rather than as child processes. Run help in st3sh for details on these.

### Quoting & Word Expansion

st3sh2 handles the POSIX quoting rules and a core set of expansions.

### POSIX Compliance

st3sh2 targets the [POSIX.1-2017 Shell Command Language](https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html) specification. The implementation was written to pass the standard's requirements for command execution order, redirection semantics, quoting rules, and exit status propagation — not just to "work like bash."

Most features to satisfy this standard have been implemented. See the toDo list for the reamining list of unfinished features.

---

## libpty — The Pseudo-Terminal

### Multithreaded Architecture

`libpty` manages PTY master/slave pairs and the I/O routing between them using a dedicated thread model. Rather than relying on `select`/`poll` loops in the main thread, the library spins up background threads to handle:

- **Reader thread** — Continuously reads from the PTY master and forwards data to the application layer
- **Writer thread** — Drains a write queue and sends data to the master without blocking the caller
- **Synchronization** — Thread-safe queues and condition variables coordinate between producer and consumer threads cleanly

This means the calling application never blocks on terminal I/O — reads and writes are non-blocking from the caller's perspective.

MIT License. See [LICENSE](LICENSE) for details.
