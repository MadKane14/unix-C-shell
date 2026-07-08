#include "jobs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

static JobProcess *head = NULL;
static int next_job_number = 1;

void add_bg_job(pid_t pid, const char *cmd_name) {
    JobProcess *new_job = malloc(sizeof(JobProcess));
    new_job->pid = pid;
    new_job->job_number = next_job_number++;
    strncpy(new_job->command_name, cmd_name, sizeof(new_job->command_name));
    new_job->next = NULL;

    if (head == NULL) {
        head = new_job;
    } else {
        JobProcess *curr = head;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = new_job;
    }
    
    // Print [job_number] pid
    printf("[%d] %d\n", new_job->job_number, new_job->pid);
}

void check_bg_jobs() {
    JobProcess *curr = head;
    JobProcess *prev = NULL;

    while (curr != NULL) {
        int status;
        // WNOHANG means "return immediately, don't wait if the process is still running"
        pid_t result = waitpid(curr->pid, &status, WNOHANG);

        if (result > 0) {
            // The process has finished!
            if (WIFEXITED(status)) {
                printf("%s with pid %d exited normally\n", curr->command_name, curr->pid);
            } else {
                printf("%s with pid %d exited abnormally\n", curr->command_name, curr->pid);
            }

            // Remove it from the linked list to prevent a memory leak
            if (prev == NULL) {
                head = curr->next;
                free(curr);
                curr = head;
            } else {
                prev->next = curr->next;
                free(curr);
                curr = prev->next;
            }
        } else {
            // Process is still running, move to the next one
            prev = curr;
            curr = curr->next;
        }
    }
}

// Helper comparator for qsort to sort by command name lexicographically
static int compare_jobs(const void *a, const void *b) {
    JobProcess *jobA = *(JobProcess **)a;
    JobProcess *jobB = *(JobProcess **)b;
    return strcmp(jobA->command_name, jobB->command_name);
}

void execute_activities(Command *cmd) {
    // 1. Count the jobs
    int count = 0;
    JobProcess *curr = head;
    while (curr != NULL) { 
        count++; 
        curr = curr->next; 
    }

    if (count == 0) return;

    // 2. Extract into an array for sorting
    JobProcess **job_array = malloc(count * sizeof(JobProcess *));
    curr = head;
    for (int i = 0; i < count; i++) {
        job_array[i] = curr;
        curr = curr->next;
    }

    // 3. Sort lexicographically
    qsort(job_array, count, sizeof(JobProcess *), compare_jobs);

    // 4. Print results
    for (int i = 0; i < count; i++) {
        printf("[%d] : %s - Running\n", job_array[i]->pid, job_array[i]->command_name);
    }

    free(job_array);
}