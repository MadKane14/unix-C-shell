#include "prompt.h"
#include "parser.h"
#include "executor.h"
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

        input_buffer[strcspn(input_buffer, "\n")] = 0;

        if (strlen(input_buffer) == 0) {
            continue;
        }

        Job current_job = parse_input(input_buffer);

        if (!current_job.is_valid) {
            printf("Invalid Syntax!\n");
            continue;
        }

        // Send the parsed job to the executor
        execute_job(&current_job);
    }

    return 0;
}