#ifndef LOG_H
#define LOG_H

#include "parser.h"

// Load history from the file on shell startup
void init_log();

// Evaluate and save the command to the log file (if it meets the rules)
void save_to_log(const char *raw_input, Job *job);

// Execute the log builtin commands (log, log purge, log execute)
void execute_log_builtin(Command *cmd);

#endif 