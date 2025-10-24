/*
 * B-Series Tree Expression Lexer for DTESN Kernel
 * ===============================================
 * 
 * Lex/Flex lexer for B-series rooted tree expressions.
 * Tokenizes bracket notation and tree operators for yacc parser.
 * 
 * Supports tree notation: [], [t], [t1, t2, ...], with
 * property queries like order(t), alpha(t), gamma(t).
 */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bseries_parser.tab.h"

/* Track line numbers for error reporting */
int yylineno = 1;
%}

/* Lexer options */
%option noyywrap
%option case-insensitive

/* Regular expression definitions */
DIGIT           [0-9]
LETTER          [a-zA-Z]
ALPHANUM        [a-zA-Z0-9_]
WHITESPACE      [ \t\r]
NEWLINE         \n

%%

    /* Comments */
"#".*$                      { /* Single-line comment - ignore */ }
"//".*$                     { /* Alternative comment style */ }

    /* Keywords - tree properties and operators */
"order"                     { return ORDER; }
"alpha"                     { return ALPHA; }
"gamma"                     { return GAMMA; }
"beta"                      { return BETA; }
"density"                   { return DENSITY; }
"symmetry"                  { return SYMMETRY; }
"empty"                     { return EMPTY_TREE; }

    /* Operators and delimiters */
"["                         { return LBRACKET; }
"]"                         { return RBRACKET; }
"("                         { return LPAREN; }
")"                         { return RPAREN; }
","                         { return COMMA; }
";"                         { return SEMICOLON; }
"."                         { return DOT; }

    /* Numbers */
{DIGIT}+                    {
                                yylval.integer = atoi(yytext);
                                return INTEGER;
                            }

{DIGIT}+"."{DIGIT}*         {
                                yylval.real = atof(yytext);
                                return REAL;
                            }

{DIGIT}*"."{DIGIT}+         {
                                yylval.real = atof(yytext);
                                return REAL;
                            }

    /* Whitespace */
{WHITESPACE}+               { /* Ignore whitespace */ }

{NEWLINE}                   {
                                yylineno++;
                            }

    /* Any other character is an error */
.                           {
                                fprintf(stderr, "Unexpected character '%s' at line %d\n",
                                       yytext, yylineno);
                            }

%%

/*
 * B-Series Lexer Utility Functions
 * =================================
 */

/**
 * bseries_lex_init - Initialize lexer with source string
 * @source: B-series tree expression to tokenize
 * 
 * Sets up the lexer to read from a string buffer.
 */
void bseries_lex_init(const char *source) {
    if (!source) {
        return;
    }
    
    /* In a real implementation, would use yy_scan_string() */
    yylineno = 1;
}

/**
 * bseries_lex_cleanup - Clean up lexer resources
 */
void bseries_lex_cleanup(void) {
    /* In a real implementation, would call yylex_destroy() */
}

/**
 * bseries_lex_reset - Reset lexer state
 */
void bseries_lex_reset(void) {
    yylineno = 1;
}
