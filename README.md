# POSIX-Compliant C Shell

A custom, POSIX-compliant Unix shell built entirely from scratch in C. This project demonstrates low-level operating system interactions, including process management, inter-process communication (IPC), file descriptor manipulation, and custom signal handling.

## 🚀 Core Features

### 1. Process & Job Control
* **Foreground & Background Execution:** Supports standard execution and asynchronous background processing using the `&` operator.
* **Process Groups:** Safely isolates pipeline processes into dedicated process groups (`setpgid`) to prevent terminal crashes.
* **Signal Handling:** Custom interceptors for `SIGINT` (Ctrl-C) and `SIGTSTP` (Ctrl-Z). Signals are correctly routed only to the foreground process group, leaving the shell and background jobs alive.
* **Job Tracking:** Tracks job states (Running/Stopped) and asynchronously reaps zombie processes using `waitpid` with `WNOHANG` and `WUNTRACED`.

### 2. Inter-Process Communication (IPC) & Redirection
* **Pipelining (`|`):** Supports chaining an arbitrary number of commands together using `pipe()`. 
* **I/O Redirection (`<`, `>`, `>>`):** Manipulates standard input/output streams using `dup2()`, seamlessly integrating with pipelines.

### 3. Built-in Intrinsics
Custom implementations of standard Unix utilities that execute directly within the shell's memory space:
* `hop`: Changes the current working directory, tracking relative paths and the previous directory (`-`).
* `reveal`: A custom implementation of `ls` that reads directory streams (`opendir`/`readdir`) and sorts output lexicographically.
* `log`: A persistent, circular-buffer command history that saves across shell sessions.
* `activities`: Lists all processes currently spawned by the shell, displaying their PID, command name, and state.
* `ping`: Sends specific signals to process IDs using `kill()`.
* `fg` / `bg`: Moves stopped or background jobs into the foreground or background by sending `SIGCONT`.

## 🏗️ Architecture

The codebase is highly modular, avoiding monolithic anti-patterns:
* **`parser.c`**: A custom lexical parser utilizing POSIX Regular Expressions (`<regex.h>`) to validate syntax and tokenize input.
* **`executor.c`**: The execution engine handling `fork()`, `execvp()`, file descriptor routing, and process group management.
* **`jobs.c`**: A linked-list data structure for tracking background jobs and reaping exit statuses.
* **`signals.c`**: Global signal interceptors and foreground process tracking.
* **`builtins.c`**: The routing logic for shell-native commands.

## 🛠️ Build & Run Instructions

This shell requires a POSIX-compliant environment (Linux, macOS, or Windows WSL) and GCC.

1. Clone the repository:
   ```bash
   git clone [https://github.com/YourUsername/Your-Repo-Name.git](https://github.com/YourUsername/Your-Repo-Name.git)
   cd shell

2. Compile the project using the provided strict Makefile:

   ```bash
   make all
   ```

3. Launch the shell:

   ```bash
   ./shell.out
   ```