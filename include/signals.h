#ifndef SIGNALS_H
#define SIGNALS_H

#include <sys/types.h>

// Global variable to track the foreground process group
extern pid_t fg_pgid;

void setup_signals();

#endif 