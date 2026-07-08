#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>
#include "parser.h"

typedef enum { RUNNING, STOPPED } JobState;

typedef struct JobProcess {
    pid_t pid; // Acts as the Process Group ID for pipelines
    int job_number;
    char command_name[4096];
    JobState state;
    struct JobProcess *next;
} JobProcess;

void add_bg_job(pid_t pid, const char *cmd_name, JobState state);
void check_bg_jobs();
void execute_activities(Command *cmd);

JobProcess* get_job_by_number(int job_number);
JobProcess* get_most_recent_job();
void remove_job_by_pid(pid_t pid);
void kill_all_jobs();

#endif 