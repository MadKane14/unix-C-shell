#include "prompt.h"
#include <stdio.h>
#include <stdbool.h>

int main() {
    char input_buffer[4096];

    // The core REPL (Read-Eval-Print Loop)
    while (true) {
        display_prompt();
        
        // Temporarily handling Part A.2 just to keep the loop from spinning infinitely
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
            // If fgets returns NULL, we hit EOF (Ctrl+D)
            printf("\n");
            break; 
        }
    }

    return 0;
}