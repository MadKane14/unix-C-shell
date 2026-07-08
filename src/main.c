#include "prompt.h"
#include "parser.h"
#include "executor.h"
#include "log.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "jobs.h"

int main() {
    char input_buffer[4096];
    char original_input[4096]; // To save the unmodified string for the log

    init_log(); // Load history from disk on startup

    while (true) {
        // Harvest any background zombies and print their exit status!
        check_bg_jobs();
        display_prompt();
        
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
            printf("\nlogout\n");
            break; 
        }

        input_buffer[strcspn(input_buffer, "\n")] = 0;

        if (strlen(input_buffer) == 0) {
            continue;
        }

        // Save a copy of the exact string before the parser mangles it
        strcpy(original_input, input_buffer);

        Job current_job = parse_input(input_buffer);

        if (!current_job.is_valid) {
            printf("Invalid Syntax!\n");
            continue;
        }

        // Save to log BEFORE executing (passing the original string)
        save_to_log(original_input, &current_job);

        execute_job(&current_job);
    }

    return 0;
}