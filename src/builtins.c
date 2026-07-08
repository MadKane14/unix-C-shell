#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <limits.h>

// Global state to track the previous directory for "hop -"
static char previous_dir[PATH_MAX] = "";

void execute_hop(Command *cmd) {
    char current_dir[PATH_MAX];
    char target_dir[PATH_MAX];

    // Get the home directory
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    char *home_dir = pw->pw_dir;

    // If no arguments, default to home directory
    if (cmd->arg_count == 1) {
        getcwd(current_dir, sizeof(current_dir));
        if (chdir(home_dir) == 0) {
            strncpy(previous_dir, current_dir, sizeof(previous_dir));
        } else {
            perror("hop");
        }
        return;
    }

    // Execute hop sequentially for each argument
    for (int i = 1; i < cmd->arg_count; i++) {
        char *arg = cmd->args[i];
        getcwd(current_dir, sizeof(current_dir));

        if (strcmp(arg, "~") == 0) {
            strncpy(target_dir, home_dir, sizeof(target_dir));
        } else if (strcmp(arg, "-") == 0) {
            if (strlen(previous_dir) == 0) {
                // If there is no previous directory yet, do nothing
                continue; 
            }
            strncpy(target_dir, previous_dir, sizeof(target_dir));
        } else {
            strncpy(target_dir, arg, sizeof(target_dir));
        }

        // Attempt to change directory
        if (chdir(target_dir) == 0) {
            // Success: update previous directory
            strncpy(previous_dir, current_dir, sizeof(previous_dir));
            // Optional: Print absolute path of new directory (often helpful for debugging)
            // char new_cwd[PATH_MAX];
            // getcwd(new_cwd, sizeof(new_cwd));
            // printf("%s\n", new_cwd);
        } else {
            printf("No such directory!\n");
        }
    }
}

bool execute_builtin(Command *cmd) {
    if (strcmp(cmd->name, "hop") == 0) {
        execute_hop(cmd);
        return true;
    }
    // Future built-ins (reveal, log, activities, etc.) will go here
    
    return false;
}