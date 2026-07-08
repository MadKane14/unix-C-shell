#include "signals.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

pid_t fg_pgid = -1;

void sigint_handler(int sig) {
    if (fg_pgid != -1) {
        // Send SIGINT to the entire foreground process group (negative PID)
        kill(-fg_pgid, SIGINT);
    } else {
        printf("\n"); 
    }
}

void sigtstp_handler(int sig) {
    if (fg_pgid != -1) {
        // Send SIGTSTP to the entire foreground process group
        kill(-fg_pgid, SIGTSTP);
    } else {
        printf("\n");
    }
}

void setup_signals() {
    struct sigaction sa;
    sa.sa_flags = SA_RESTART; // Restart interrupted system calls (like fgets)
    sigemptyset(&sa.sa_mask);

    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);

    sa.sa_handler = sigtstp_handler;
    sigaction(SIGTSTP, &sa, NULL);

    // Ignore these signals so background jobs don't crash when trying to read/write
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
}