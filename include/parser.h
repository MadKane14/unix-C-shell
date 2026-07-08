#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

#define MAX_ARGS 64
#define MAX_COMMANDS 16 // Max commands in a single pipeline

// Represents a single command (e.g., "cat meow.txt" or "grep dog")
typedef struct {
    char *name;                 // e.g., "cat"
    char *args[MAX_ARGS];       // e.g., ["cat", "meow.txt", NULL] (Execvp needs NULL termination)
    int arg_count;
    char *input_file;           // "< file"
    char *output_file;          // "> file" or ">> file"
    bool append_output;         // True if ">>"
} Command;

// Represents an entire job line inputted by the user
typedef struct {
    Command commands[MAX_COMMANDS];
    int command_count;
    bool run_in_background;     // True if line ends with '&'
    bool is_valid;              // False if it violates the CFG syntax
} Job;

// Core function to parse the raw string into our Job struct
Job parse_input(char *raw_input);

// Debugging function to print the parsed structure
void print_job(Job job);

#endif // PARSER_H