#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>
#include "parser.h"

// A node in our linked list of background processes
typedef struct JobProcess {
    pid_t pid;
    int job_number;
    char command_name[4096];
    struct JobProcess *next;
} JobProcess;

// Add a new process to the background tracker
void add_bg_job(pid_t pid, const char *cmd_name);

// Check if any background jobs have finished
void check_bg_jobs();

// Built-in command to list running processes
void execute_activities(Command *cmd);

#endif 