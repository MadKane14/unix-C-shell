#ifndef BUILTINS_H
#define BUILTINS_H

#include "parser.h"
#include <stdbool.h>

// Returns true if the command was a built-in and executed, false otherwise
bool execute_builtin(Command *cmd);

// Specific built-in implementations
void execute_hop(Command *cmd);

void execute_reveal(Command *cmd);

#endif 