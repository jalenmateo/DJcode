%{
#define YY_NO_UNISTD_H 1
#define isatty(x) 0  
#include <stdio.h>
%}
%option noinput nounput
%%

"Pattern"              { printf("PATTERN\n"); }
":"                    { printf("COLON\n"); }
[0-9]+                 { printf("NUMBER %s\n", yytext); }
"Drum"|"Triangle"      { printf("INSTRUMENT %s\n", yytext); }
"boom"|"tsst"|"clap"|"dun"|"ding"|"diding"|"dididing"|"crash"|"rest"  { printf("INSTRUMENT_SOUND %s\n", yytext); }
"Drop the beat"        { printf("MAIN\n");}
"Play"                 { printf("PLAY\n");}
"x"                    { printf("LOOP\n");}
[ \t\n]+               { /* ignore whitespace */ }
.                      { /* ignore other characters */ }

%%

int yywrap(void) {
    return 1;
}
