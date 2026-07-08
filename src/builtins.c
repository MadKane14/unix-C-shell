#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <limits.h>
#include <dirent.h>

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

static int compare_strings(const void *a, const void *b) {
    const char *str_a = *(const char **)a;
    const char *str_b = *(const char **)b;
    return strcmp(str_a, str_b);
}

void execute_reveal(Command *cmd) {
    bool show_hidden = false;
    bool show_line = false;
    char target_dir[PATH_MAX] = "."; // Default to current directory
    bool path_provided = false;

    // 1. Parse Flags and Arguments
    for (int i = 1; i < cmd->arg_count; i++) {
        char *arg = cmd->args[i];
        
        if (arg[0] == '-' && strlen(arg) > 1 && strcmp(arg, "-") != 0) {
            // It's a flag string 
            for (int j = 1; arg[j] != '\0'; j++) {
                if (arg[j] == 'a') show_hidden = true;
                else if (arg[j] == 'l') show_line = true;
                else {
                    printf("reveal: Invalid Syntax!\n");
                    return;
                }
            }
        } else {
            // It's a path argument
            if (path_provided) {
                printf("reveal: Invalid Syntax!\n"); // Too many arguments
                return;
            }
            path_provided = true;
            
            // Resolve the path identical to 'hop'
            if (strcmp(arg, "~") == 0) {
                uid_t uid = geteuid();
                struct passwd *pw = getpwuid(uid);
                strncpy(target_dir, pw->pw_dir, sizeof(target_dir));
            } else if (strcmp(arg, "-") == 0) {
                if (strlen(previous_dir) == 0) {
                    printf("No such directory!\n");
                    return;
                }
                strncpy(target_dir, previous_dir, sizeof(target_dir));
            } else {
                strncpy(target_dir, arg, sizeof(target_dir));
            }
        }
    }

    // Handle default "hop" behavior if no path is provided
    if (!path_provided) {
        uid_t uid = geteuid();
        struct passwd *pw = getpwuid(uid);
        strncpy(target_dir, pw->pw_dir, sizeof(target_dir));
    }

    // 2. Open Directory
    DIR *dir = opendir(target_dir);
    if (dir == NULL) {
        printf("No such directory!\n");
        return;
    }

    // 3. Read Directory Contents into an Array
    struct dirent *entry;
    char *files[2048]; 
    int count = 0;

    while ((entry = readdir(dir)) != NULL) {
        // Filter out hidden files unless -a is passed
        if (!show_hidden && entry->d_name[0] == '.') {
            continue;
        }
        
        files[count] = strdup(entry->d_name);
        count++;
    }
    closedir(dir);

    // 4. Sort Lexicographically
    qsort(files, count, sizeof(char *), compare_strings);

    // 5. Print Output
    for (int i = 0; i < count; i++) {
        if (show_line) {
            printf("%s\n", files[i]);
        } else {
            printf("%s  ", files[i]);
        }
        free(files[i]); // Prevent memory leaks
    }
    
    if (!show_line && count > 0) {
        printf("\n"); // Print trailing newline if not line-by-line
    }
}

bool execute_builtin(Command *cmd) {
    if (strcmp(cmd->name, "hop") == 0) {
        execute_hop(cmd);
        return true;
    } else if (strcmp(cmd->name, "reveal") == 0) {
        execute_reveal(cmd);
        return true;
    }
    return false;
}