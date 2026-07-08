#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

// Helper function to trim leading and trailing whitespace
static char *trim_whitespace(char *str) {
    char *end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

// POSIX Regex Validator
static bool is_valid_syntax(const char *input) {
    regex_t regex;
    int reti;
    
    const char *invalid_pattern = 
        "^[[:space:]]*[|&;]"                // Starts with |, &, or ;
        "|"
        "[|&;][[:space:]]*[|&;]"            // Consecutive operators like | | or ; ;
        "|"
        "[><][[:space:]]*[|&;]"             // Redirection followed by an operator like > |
        "|"
        "[><][[:space:]]*$"                 // Redirection at the very end with no file
        "|"
        "[|][[:space:]]*$";                 

    // Compile the regex using Extended Regular Expressions 
    reti = regcomp(&regex, invalid_pattern, REG_EXTENDED | REG_NOSUB);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return false;
    }

    // Execute the regex. If it matches, we found INVALID syntax.
    reti = regexec(&regex, input, 0, NULL, 0);
    regfree(&regex);

    if (!reti) {
        return false; // Match found -> Syntax is invalid
    } else if (reti == REG_NOMATCH) {
        return true;  // No match -> Syntax is valid
    } else {
        return false; // Regex execution error
    }
}

Job parse_input(char *raw_input) {
    Job job = {0};
    job.is_valid = true;

    // 1. Clean the input
    char *cleaned = trim_whitespace(raw_input);
    if (strlen(cleaned) == 0) {
        job.is_valid = false;
        return job; 
    }

    // 2. Strict POSIX Regex Validation!
    if (!is_valid_syntax(cleaned)) {
        job.is_valid = false;
        return job;
    }

    // 3. Check for background execution flag '&'
    if (cleaned[strlen(cleaned) - 1] == '&') {
        job.run_in_background = true;
        cleaned[strlen(cleaned) - 1] = '\0'; 
        cleaned = trim_whitespace(cleaned);  
    }

    // 4. Tokenize by pipe '|' 
    char *saveptr_pipe;
    char *cmd_token = strtok_r(cleaned, "|", &saveptr_pipe);
    
    while (cmd_token != NULL) {
        if (job.command_count >= MAX_COMMANDS) break;

        // Initialize the cmd and trimmed_cmd variables here!
        Command *cmd = &job.commands[job.command_count];
        char *trimmed_cmd = trim_whitespace(cmd_token);

        // 5. Tokenize the individual command by space to get arguments
        char *saveptr_space;
        char *arg_token = strtok_r(trimmed_cmd, " \t", &saveptr_space);
        
        while (arg_token != NULL) {
            if (strncmp(arg_token, ">>", 2) == 0) {
                cmd->append_output = true;
                // Handle both ">>file" and ">> file"
                if (strlen(arg_token) > 2) cmd->output_file = strdup(arg_token + 2);
                else { 
                    arg_token = strtok_r(NULL, " \t", &saveptr_space); 
                    if (arg_token) cmd->output_file = strdup(arg_token); 
                }
            } else if (arg_token[0] == '>') {
                cmd->append_output = false;
                // Handle both ">file" and "> file"
                if (strlen(arg_token) > 1) cmd->output_file = strdup(arg_token + 1);
                else { 
                    arg_token = strtok_r(NULL, " \t", &saveptr_space); 
                    if (arg_token) cmd->output_file = strdup(arg_token); 
                }
            } else if (arg_token[0] == '<') {
                // Handle both "<file" and "< file"
                if (strlen(arg_token) > 1) cmd->input_file = strdup(arg_token + 1);
                else { 
                    arg_token = strtok_r(NULL, " \t", &saveptr_space); 
                    if (arg_token) cmd->input_file = strdup(arg_token); 
                }
            } else {
                // It's a standard argument/command name
                if (cmd->arg_count == 0) {
                    cmd->name = strdup(arg_token);
                }
                cmd->args[cmd->arg_count++] = strdup(arg_token);
            }
            arg_token = strtok_r(NULL, " \t", &saveptr_space);
        }
        cmd->args[cmd->arg_count] = NULL; // execvp requires a NULL terminated array!
        
        job.command_count++;
        cmd_token = strtok_r(NULL, "|", &saveptr_pipe);
    }

    return job;
}