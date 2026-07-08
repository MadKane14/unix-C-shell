#include "prompt.h"
#include "parser.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int main() {
    char input_buffer[4096];

    while (true) {
        display_prompt();
        
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
            printf("\nlogout\n");
            break; 
        }

        // Remove the trailing newline character from fgets
        input_buffer[strcspn(input_buffer, "\n")] = 0;

        // Skip empty inputs (just pressing enter)
        if (strlen(input_buffer) == 0) {
            continue;
        }

        // Parse the input
        Job current_job = parse_input(input_buffer);

        // Part A.3 requirement: Print Invalid Syntax!
        if (!current_job.is_valid) {
            printf("Invalid Syntax!\n");
            continue;
        }

        // For now, let's just print what we parsed to verify it works!
        for (int i = 0; i < current_job.command_count; i++) {
            printf("[Debug] Command %d: %s\n", i, current_job.commands[i].name);
            for (int j = 0; j < current_job.commands[i].arg_count; j++) {
                printf("  - Arg %d: %s\n", j, current_job.commands[i].args[j]);
            }
        }
        if (current_job.run_in_background) {
            printf("[Debug] Job marked to run in background (&)\n");
        }
    }

    return 0;
}