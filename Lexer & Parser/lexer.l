%{
#include <stdio.h>
%}

%%

"Pattern"              { printf("PATTERN\n"); }
":"                    { printf("COLON\n"); }
[0-9]+                 { printf("NUMBER %s\n", yytext); }
"Drum"|"Triangle"      { printf("INSTRUMENT %s\n", yytext); }
"boom"|"clap"|"ting"   { printf("INSTRUMENT_SOUND %s\n", yytext); }
"Drop the beat"        { printf("MAIN\n");}
"Play"                 { printf("PLAY\n");}
"x"                    { printf("LOOP\n");}
[ \t\n]+               { /* ignore whitespace */ }
.                      { /* ignore other characters */ }

%%

int yywrap(void) {
    return 1;
}
