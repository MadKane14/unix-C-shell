#include "executor.h"
#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include "jobs.h"
#include "signals.h" // FIXED: Plural "signals.h"

void execute_job(Job *job) {
    if (!job->is_valid || job->command_count == 0) {
        return;
    }

    // Fast path: If it's a single built-in command
    if (job->command_count == 1 && (strcmp(job->commands[0].name, "hop") == 0 || strcmp(job->commands[0].name, "reveal") == 0)) {
        Command *cmd = &job->commands[0];
        
        int saved_stdout = dup(STDOUT_FILENO);
        
        if (cmd->output_file) {
            int flags = O_WRONLY | O_CREAT | (cmd->append_output ? O_APPEND : O_TRUNC);
            int fd = open(cmd->output_file, flags, 0644);
            if (fd >= 0) { dup2(fd, STDOUT_FILENO); close(fd); }
        }

        execute_builtin(cmd);
        
        fflush(stdout); 
        
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
        return;
    }

    int prev_pipe = -1;
    pid_t pids[MAX_COMMANDS];

    // Loop through the pipeline
    for (int i = 0; i < job->command_count; i++) {
        Command *cmd = &job->commands[i];
        int pipe_fd[2];

        // Create a pipe for all commands EXCEPT the very last one
        if (i < job->command_count - 1) {
            if (pipe(pipe_fd) < 0) {
                perror("pipe");
                return;
            }
        }

        pids[i] = fork();
        
        if (pids[i] < 0) {
            perror("fork");
        } else if (pids[i] == 0) {
            // =================CHILD PROCESS=================
            // Reset signals to default so the child CAN be killed/stopped
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            // Put the child in a new process group. 
            if (i == 0) setpgid(0, 0); 
            else setpgid(0, pids[0]);

            // 1. Read from the previous pipe 
            if (prev_pipe != -1) {
                dup2(prev_pipe, STDIN_FILENO);
                close(prev_pipe);
            }

            // 2. Write to the current pipe
            if (i < job->command_count - 1) {
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[0]); 
                close(pipe_fd[1]);
            }

            // 3. Explicit File Redirections
            if (cmd->input_file) {
                int fd = open(cmd->input_file, O_RDONLY);
                if (fd < 0) {
                    printf("No such file or directory\n");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            if (cmd->output_file) {
                int flags = O_WRONLY | O_CREAT | (cmd->append_output ? O_APPEND : O_TRUNC);
                int fd = open(cmd->output_file, flags, 0644); 
                if (fd < 0) {
                    printf("Unable to create file for writing\n");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            // 4. Execute the command
            if (execute_builtin(cmd)) {
                exit(EXIT_SUCCESS); 
            } else if (execvp(cmd->name, cmd->args) == -1) {
                printf("Command not found!\n");
                exit(EXIT_FAILURE);
            }
            
        } else {
            // =================PARENT PROCESS=================
            // Parent also calls setpgid to avoid race conditions with the child
            if (i == 0) {
                setpgid(pids[0], pids[0]);
                if (!job->run_in_background) fg_pgid = pids[0]; // Mark as foreground!
            } else {
                setpgid(pids[i], pids[0]);
            }
            
            // Close the read end of the previous pipe
            if (prev_pipe != -1) {
                close(prev_pipe);
            }
            // Save the read end of the current pipe for the NEXT child
            if (i < job->command_count - 1) {
                close(pipe_fd[1]); 
                prev_pipe = pipe_fd[0]; 
            }
        }
    }

    // Wait for all children in the pipeline to finish
    if (!job->run_in_background) {
        int status;
        for (int i = 0; i < job->command_count; i++) {
            // WUNTRACED allows waitpid to return if the process is paused by Ctrl-Z
            waitpid(pids[i], &status, WUNTRACED);
            
            // If the group leader was paused by Ctrl-Z, add it to the background tracker!
            if (i == 0 && WIFSTOPPED(status)) {
                add_bg_job(pids[0], job->commands[0].name, STOPPED);
            }
        }
        fg_pgid = -1; // Reset foreground tracking
    } else {
        add_bg_job(pids[0], job->commands[0].name, RUNNING);
    }
}