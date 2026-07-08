#include "executor.h"
#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void execute_job(Job *job) {
    if (!job->is_valid || job->command_count == 0) {
        return;
    }

    // For Checkpoint 3, we are only handling the first command in the pipeline
    Command *cmd = &job->commands[0];

    // 1. Check if it's a built-in (like hop). If so, run it in the main shell process.
    if (execute_builtin(cmd)) {
        return;
    }

    // 2. If it's an external command, we must fork a new process.
    pid_t pid = fork();

    if (pid < 0) {
        // Fork failed
        perror("fork");
    } else if (pid == 0) {
        // CHILD PROCESS
        
        // execvp searches the PATH environment variable to find the binary.
        // If it succeeds, the child process is completely replaced by the new program.
        // If it fails, it returns -1.
        if (execvp(cmd->name, cmd->args) == -1) {
            printf("Command not found!\n");
            exit(EXIT_FAILURE); // Kill the child process so it doesn't run the shell loop
        }
    } else {
        // PARENT PROCESS (The Shell)
        
        if (!job->run_in_background) {
            // Foreground task: Shell waits for the child to finish
            int status;
            waitpid(pid, &status, 0);
        } else {
            // Background task (&): Shell does NOT wait.
            // Note: Proper background job tracking (printing PID on start/exit) is required for Part D.
            // This is a minimal implementation to prevent blocking.
            printf("[Debug] Background process started with PID %d\n", pid);
        }
    }
}