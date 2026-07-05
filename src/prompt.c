#include "prompt.h"
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>

void display_prompt() {
    char hostname[HOST_NAME_MAX];
    char cwd[PATH_MAX];
    
    // Fetch user details
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    
    // Fetch system hostname and current working directory
    gethostname(hostname, sizeof(hostname));
    getcwd(cwd, sizeof(cwd));

    char *home_dir = pw->pw_dir;
    char display_path[PATH_MAX];

    // Check if the current working directory is inside the home directory
    if (strncmp(cwd, home_dir, strlen(home_dir)) == 0) {
        // Replace the home_dir part with '~'
        snprintf(display_path, sizeof(display_path), "~%s", cwd + strlen(home_dir));
    } else {
        // If outside home directory, display absolute path
        snprintf(display_path, sizeof(display_path), "%s", cwd);
    }

    // Print the prompt. 
    printf("<%s@%s:%s> ", pw->pw_name, hostname, display_path);
    
    // We must flush stdout because there is no newline character (\n) at the end of the prompt.
    // Without this, the OS might buffer the output and not display it immediately.
    fflush(stdout); 
}