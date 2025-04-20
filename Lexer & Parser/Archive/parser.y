%{
#include <stdio.h>
#include <stdlib.h>

// Declare yylex and yyerror to fix compiler errors
int yylex(void);
int yyerror(const char *s);
%}

%token PATTERN INSTRUMENT INSTRUMENT_SOUND NUMBER COLON

%%

program
    : pattern_block
    ;

pattern_block
    : named_pattern
    | pattern_block named_pattern
    ;

named_pattern
    : PATTERN NUMBER COLON instrument_sequence
    ;

instrument_sequence
    : instrument_sound_group 
    | instrument_sequence instrument_sound_group
    ;

instrument_sound_group
    : INSTRUMENT sound_sequence
    ;

sound_sequence
    : INSTRUMENT_SOUND
    | sound_sequence INSTRUMENT_SOUND
    ;

%%

int yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s\n", s);
    return 1;
}