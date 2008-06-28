
%{
#include <stdio.h>
#include <string>
#include "storage.hh"

#include "parser2.tab.h"
%}

%%

::                      return PROPERTY;
:=                      return ASSIGN;
\!=                     return UNEQUALS;
\=                      return EQUALS;
[ ]*[*][ ]*             return PROD;
[ ]*[+][ ]*             return SUM;
\{[ ]*                  return LCURLY;
[ ]*\}                  return RCURLY;
\([ ]*                  return LROUND;
[ ]*\)                  return RROUND;
\[[ ]*                  return LSQUARE;
[ ]*\]                  return RSQUARE;
\.\.                    return SEQUENCE;
_                       return UNDERSCORE;
[\^]                    return CARET;
[ ]*[;][ ]*             return SEMICOLON;
[ ]*[,][ ]*             return COMMA;
-\>                     return RIGHTARROW;
\|                      return CONDITIONAL;
\!                      return FACTORIAL;

[0-9]+([ \n\t]*[/][0-9]+)?   { yylval.str=new std::string(yytext); return NUMBER; }
\\[a-zA-Z]+[a-zA-Z0-9']* { yylval.str=new std::string(yytext); return TEXNAME; }
@[a-zA-Z][a-zA-Z]*       { yylval.str=new std::string(yytext); return COMMAND; }
[a-zA-Z]+[a-zA-Z0-9']*   { yylval.str=new std::string(yytext); return NAME; }
[ \n\t]+                 return WHITE;

%%
