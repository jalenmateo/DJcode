#ifndef TOKENSPARSER_H
#define TOKENSPARSER_H

#include <stdint.h> /* For int16_t if needed, though not directly used here*/

#define MAX_PATTERNS 4        /* Allow up to 4 pattern definitions only for now*/
#define MAX_SOUNDS_PER_PATTERN 8  /* Allow max 8 sounds per pattern*/
#define MAX_PLAY_COMMANDS 10 /* Allow up to 10 play commands*/
#define MAX_NAME_LEN 16       /* Max length for pattern and sound names*/

/* Struct to hold a single pattern definition*/
typedef struct {
    char name[MAX_NAME_LEN];
    char sounds[MAX_SOUNDS_PER_PATTERN][MAX_NAME_LEN];
    int num_sounds;
} Pattern;

/* Struct to hold a single play command*/
typedef struct {
    char pattern_name[MAX_NAME_LEN];
    int loop_count;
} PlayCommand;

/* Reads the token file and populates the patterns and play sequence arrays.*/
/* Returns 0 on success, -1 on file error, -2 on parsing error (e.g., limits exceeded).*/
int parse_tokens_file(const char* filename,
                      Pattern patterns[MAX_PATTERNS],
                      int* num_patterns,
                      PlayCommand play_sequence[MAX_PLAY_COMMANDS],
                      int* num_play_commands);

#endif
