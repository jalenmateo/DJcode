#include "tokensParser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 256 // Maximum length of a line in the tokens file

// Function implementation for the parser
int parse_tokens_file(const char* filename,
                      Pattern patterns[MAX_PATTERNS],
                      int* num_patterns,
                      PlayCommand play_sequence[MAX_PLAY_COMMANDS],
                      int* num_play_commands)
{
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Error opening tokens file");
        return -1; // File error
    }

    char line[MAX_LINE_LEN];
    *num_patterns = 0;
    *num_play_commands = 0;
    int current_pattern_index = -1; // Index of the pattern currently being defined, -1 if none

    while (fgets(line, sizeof(line), fp)) {
        // Remove trailing newline character if present
        line[strcspn(line, "\r\n")] = 0;

        // Skip empty lines or lines with only whitespace
        if (strspn(line, " \t") == strlen(line)) {
            continue;
        }

        // Check for keywords
        if (strncmp(line, "PATTERN ", 8) == 0) {
            if (current_pattern_index != -1) {
                fprintf(stderr, "Error: Nested PATTERN definition or missing END.\n");
                fclose(fp);
                return -2; // Parsing error
            }
            if (*num_patterns >= MAX_PATTERNS) {
                fprintf(stderr, "Error: Maximum number of patterns (%d) exceeded.\n", MAX_PATTERNS);
                fclose(fp);
                return -2; // Parsing error
            }

            current_pattern_index = *num_patterns;
            patterns[current_pattern_index].num_sounds = 0;

            // Extract pattern name
            if (sscanf(line, "PATTERN %31s", patterns[current_pattern_index].name) != 1) {
                 fprintf(stderr, "Error: Could not parse pattern name in line: %s\n", line);
                 fclose(fp);
                 return -2;
            }
            (*num_patterns)++;

        } else if (strcmp(line, "END") == 0) {
            if (current_pattern_index == -1) {
                fprintf(stderr, "Error: Found END outside of PATTERN definition.\n");
                fclose(fp);
                return -2; // Parsing error
            }
            current_pattern_index = -1; // End of current pattern definition

        } else if (strncmp(line, "PLAY ", 5) == 0) {
             if (current_pattern_index != -1) {
                fprintf(stderr, "Error: PLAY command inside PATTERN definition.\n");
                fclose(fp);
                return -2; // Parsing error
            }
            if (*num_play_commands >= MAX_PLAY_COMMANDS) {
                fprintf(stderr, "Error: Maximum number of play commands (%d) exceeded.\n", MAX_PLAY_COMMANDS);
                fclose(fp);
                return -2; // Parsing error
            }

            // Extract pattern name and loop count
            char loop_keyword[5]; // To read "LOOP"
            if (sscanf(line, "PLAY %31s %4s %d",
                       play_sequence[*num_play_commands].pattern_name,
                       loop_keyword,
                       &play_sequence[*num_play_commands].loop_count) != 3 ||
                strcmp(loop_keyword, "LOOP") != 0)
            {
                fprintf(stderr, "Error: Could not parse PLAY command in line: %s\n", line);
                fclose(fp);
                return -2;
            }
             if (play_sequence[*num_play_commands].loop_count <= 0) {
                 fprintf(stderr, "Error: Loop count must be positive in line: %s\n", line);
                 fclose(fp);
                 return -2;
             }

            (*num_play_commands)++;

        } else { // Assume it's a sound name within a pattern
            if (current_pattern_index == -1) {
                fprintf(stderr, "Error: Found sound name '%s' outside of PATTERN definition.\n", line);
                fclose(fp);
                return -2; // Parsing error
            }

            Pattern* current_p = &patterns[current_pattern_index];
            if (current_p->num_sounds >= MAX_SOUNDS_PER_PATTERN) {
                fprintf(stderr, "Error: Maximum sounds per pattern (%d) exceeded for pattern '%s'.\n",
                        MAX_SOUNDS_PER_PATTERN, current_p->name);
                fclose(fp);
                return -2; // Parsing error
            }

            // Copy sound name (ensure it's not empty and fits)
            if (strlen(line) == 0 || strlen(line) >= MAX_NAME_LEN) {
                 fprintf(stderr, "Error: Invalid or too long sound name '%s' in pattern '%s'.\n", line, current_p->name);
                 fclose(fp);
                 return -2;
            }
            strcpy(current_p->sounds[current_p->num_sounds], line);
            current_p->num_sounds++;
        }
    }

    if (current_pattern_index != -1) {
         fprintf(stderr, "Error: Reached end of file while still defining pattern '%s' (missing END?).\n", patterns[current_pattern_index].name);
         fclose(fp);
         return -2; // Parsing error
    }


    fclose(fp);
    return 0; // Success
}
