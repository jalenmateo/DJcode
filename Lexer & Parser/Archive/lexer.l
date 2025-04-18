%{
#include "parser.tab.h"
#include <string.h>
%}

%%

"Pattern"               { return PATTERN; }
":"                     { return COLON; }
"Drum"|"Triangle"       { return INSTRUMENT; }
"boom"|"clap"|"ting"    { return INSTRUMENT_SOUND; }
[0-9]+                  { yylval = atoi(yytext); return NUMBER; }
[ \t\n]+              { /* skip whitespace */ }
.                       { return yytext[0]; }

%%

int yywrap(void) {
    return 1;
}
