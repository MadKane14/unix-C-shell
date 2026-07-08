#include "log.h"
#include "executor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#define MAX_LOG 15
#define MAX_CMD_LEN 4096

static char history[MAX_LOG][MAX_CMD_LEN];
static int history_count = 0;
static char log_file_path[PATH_MAX];

void init_log() {
    // Determine the path where the shell was launched and hide the file (starts with .)
    if (getcwd(log_file_path, sizeof(log_file_path)) != NULL) {
        strcat(log_file_path, "/.shell_log");
    }

    // Load existing history from previous sessions
    FILE *f = fopen(log_file_path, "r");
    if (f) {
        char line[MAX_CMD_LEN];
        while (fgets(line, sizeof(line), f) && history_count < MAX_LOG) {
            line[strcspn(line, "\n")] = 0; // Strip newline
            if (strlen(line) > 0) {
                strcpy(history[history_count++], line);
            }
        }
        fclose(f);
    }
}

void save_to_log(const char *raw_input, Job *job) {
    // 1. Rule: Do not log if ANY command in the pipeline is 'log'
    for (int i = 0; i < job->command_count; i++) {
        if (job->commands[i].name != NULL && strcmp(job->commands[i].name, "log") == 0) {
            return; 
        }
    }

    // 2. Rule: Do not log if identical to the immediately previous command
    if (history_count > 0 && strcmp(raw_input, history[history_count - 1]) == 0) {
        return;
    }

    // 3. Enforce the 15-command limit (Shift everything left to drop the oldest)
    if (history_count == MAX_LOG) {
        for (int i = 1; i < MAX_LOG; i++) {
            strcpy(history[i - 1], history[i]);
        }
        strcpy(history[MAX_LOG - 1], raw_input);
    } else {
        strcpy(history[history_count++], raw_input);
    }

    // 4. Persist to disk immediately
    FILE *f = fopen(log_file_path, "w");
    if (f) {
        for (int i = 0; i < history_count; i++) {
            fprintf(f, "%s\n", history[i]);
        }
        fclose(f);
    }
}

void execute_log_builtin(Command *cmd) {
    if (cmd->arg_count == 1) {
        // Print stored commands (Oldest to Newest)
        for (int i = 0; i < history_count; i++) {
            printf("%s\n", history[i]);
        }
    } 
    else if (cmd->arg_count == 2 && strcmp(cmd->args[1], "purge") == 0) {
        // Purge the history
        history_count = 0;
        FILE *f = fopen(log_file_path, "w"); // Opening in 'w' mode truncates/empties it
        if (f) fclose(f);
    } 
    else if (cmd->arg_count == 3 && strcmp(cmd->args[1], "execute") == 0) {
        // Execute a specific command by index
        int index = atoi(cmd->args[2]);
        if (index <= 0 || index > history_count) {
            printf("log: Invalid Syntax!\n"); 
            return;
        }

        // indexed in order of newest to oldest
        int target = history_count - index;

        char cmd_to_run[MAX_CMD_LEN];
        strcpy(cmd_to_run, history[target]);

        // Parse and execute. 
        Job job = parse_input(cmd_to_run);
        if (job.is_valid) {
            execute_job(&job);
        } else {
            printf("Invalid Syntax!\n");
        }
    } 
    else {
        printf("log: Invalid Syntax!\n");
    }
}