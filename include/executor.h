#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"

// Takes a parsed Job and routes it to system calls
void execute_job(Job *job);

#endif 