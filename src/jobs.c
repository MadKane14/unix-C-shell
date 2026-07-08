#include "jobs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

static JobProcess *head = NULL;
static int next_job_number = 1;

void add_bg_job(pid_t pid, const char *cmd_name, JobState state) {
    JobProcess *new_job = malloc(sizeof(JobProcess));
    new_job->pid = pid;
    new_job->job_number = next_job_number++;
    new_job->state = state;
    strncpy(new_job->command_name, cmd_name, sizeof(new_job->command_name));
    new_job->next = NULL;

    if (head == NULL) head = new_job;
    else {
        JobProcess *curr = head;
        while (curr->next != NULL) curr = curr->next;
        curr->next = new_job;
    }
    
    if (state == RUNNING) printf("[%d] %d\n", new_job->job_number, new_job->pid);
    else printf("\n[%d] Stopped %s\n", new_job->job_number, new_job->command_name);
}

void remove_job_by_pid(pid_t pid) {
    JobProcess *curr = head, *prev = NULL;
    while (curr != NULL) {
        if (curr->pid == pid) {
            if (prev == NULL) head = curr->next;
            else prev->next = curr->next;
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

void check_bg_jobs() {
    JobProcess *curr = head;
    while (curr != NULL) {
        int status;
        // WUNTRACED catches stops, WCONTINUED catches resumes
        pid_t result = waitpid(curr->pid, &status, WNOHANG | WUNTRACED | WCONTINUED);
        
        JobProcess *next_node = curr->next; // Save next before potential deletion
        
        if (result > 0) {
            if (WIFEXITED(status)) {
                printf("%s with pid %d exited normally\n", curr->command_name, curr->pid);
                remove_job_by_pid(curr->pid);
            } else if (WIFSIGNALED(status)) {
                printf("%s with pid %d exited abnormally\n", curr->command_name, curr->pid);
                remove_job_by_pid(curr->pid);
            } else if (WIFSTOPPED(status)) {
                curr->state = STOPPED;
            } else if (WIFCONTINUED(status)) {
                curr->state = RUNNING;
            }
        }
        curr = next_node;
    }
}

JobProcess* get_job_by_number(int job_num) {
    JobProcess *curr = head;
    while (curr != NULL) {
        if (curr->job_number == job_num) return curr;
        curr = curr->next;
    }
    return NULL;
}

JobProcess* get_most_recent_job() {
    if (head == NULL) return NULL;
    JobProcess *curr = head;
    while (curr->next != NULL) curr = curr->next;
    return curr;
}

void kill_all_jobs() {
    JobProcess *curr = head;
    while (curr != NULL) {
        kill(-curr->pid, SIGKILL); // Kill the entire process group
        curr = curr->next;
    }
}

// Update activities to print the state correctly
static int compare_jobs(const void *a, const void *b) {
    JobProcess *jobA = *(JobProcess **)a;
    JobProcess *jobB = *(JobProcess **)b;
    return strcmp(jobA->command_name, jobB->command_name);
}

void execute_activities(Command *cmd) {
    int count = 0;
    JobProcess *curr = head;
    while (curr != NULL) { count++; curr = curr->next; }
    if (count == 0) return;

    JobProcess **arr = malloc(count * sizeof(JobProcess *));
    curr = head;
    for (int i = 0; i < count; i++) { arr[i] = curr; curr = curr->next; }
    qsort(arr, count, sizeof(JobProcess *), compare_jobs);

    for (int i = 0; i < count; i++) {
        printf("[%d] : %s - %s\n", arr[i]->pid, arr[i]->command_name, 
               arr[i]->state == RUNNING ? "Running" : "Stopped");
    }
    free(arr);
}