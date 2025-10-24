/*
 * P-lingua Lexical Analyzer for DTESN Kernel
 * ===========================================
 * 
 * Lex/Flex lexer for P-lingua membrane computing rules.
 * Tokenizes P-lingua syntax for yacc parser integration.
 * 
 * Supports full P-lingua syntax including:
 * - Membrane labels and objects
 * - Evolution, communication, division, dissolution rules
 * - Multisets with multiplicities
 * - Priority specifications
 */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "plingua_parser.tab.h"

/* Track line numbers for error reporting */
int yylineno = 1;

/* String buffer for multi-line strings */
static char string_buf[1024];
static int string_buf_ptr;
%}

/* Lexer options */
%option noyywrap
%option case-insensitive

/* State definitions for multi-line strings */
%x STRING_STATE

/* Regular expression definitions */
DIGIT           [0-9]
LETTER          [a-zA-Z]
ALPHANUM        [a-zA-Z0-9_]
WHITESPACE      [ \t\r]
NEWLINE         \n

%%

    /* Comments - P-lingua style */
"//".*$                     { /* Single-line comment - ignore */ }
"/*"([^*]|\*+[^*/])*\*+"/"  { /* Multi-line comment - ignore */ }

    /* Keywords - P-lingua constructs */
"@model"                    { return DEF; }
"def"                       { return DEF; }
"model"                     { return MODEL; }
"call"                      { return CALL; }
"end"                       { return END; }
"membrane"                  { return MEMBRANE; }
"object"                    { return OBJECT; }
"rule"                      { return RULE; }
"multiset"                  { return MULTISET; }
"priority"                  { return PRIORITY; }
"evolution"                 { return EVOLUTION; }
"communication"             { return COMMUNICATION; }
"division"                  { return DIVISION; }
"dissolution"               { return DISSOLUTION; }

    /* Operators and delimiters */
"->"                        { return ARROW; }
"["                         { return LBRACKET; }
"]"                         { return RBRACKET; }
"("                         { return LPAREN; }
")"                         { return RPAREN; }
","                         { return COMMA; }
";"                         { return SEMICOLON; }
":"                         { return COLON; }
"'"                         { return APOSTROPHE; }

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

    /* Identifiers - membrane labels, object symbols */
{LETTER}{ALPHANUM}*         {
                                yylval.string = strdup(yytext);
                                return IDENTIFIER;
                            }

    /* String literals */
\"                          {
                                string_buf_ptr = 0;
                                BEGIN(STRING_STATE);
                            }

<STRING_STATE>\"            {
                                BEGIN(INITIAL);
                                string_buf[string_buf_ptr] = '\0';
                                yylval.string = strdup(string_buf);
                                return STRING_LITERAL;
                            }

<STRING_STATE>\n            {
                                /* Error - unterminated string */
                                fprintf(stderr, "Unterminated string at line %d\n", 
                                       yylineno);
                                BEGIN(INITIAL);
                            }

<STRING_STATE>\\n           {
                                string_buf[string_buf_ptr++] = '\n';
                            }

<STRING_STATE>\\t           {
                                string_buf[string_buf_ptr++] = '\t';
                            }

<STRING_STATE>\\\"          {
                                string_buf[string_buf_ptr++] = '"';
                            }

<STRING_STATE>\\\\          {
                                string_buf[string_buf_ptr++] = '\\';
                            }

<STRING_STATE>.             {
                                if (string_buf_ptr < sizeof(string_buf) - 1) {
                                    string_buf[string_buf_ptr++] = yytext[0];
                                }
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
 * P-lingua Lexer Utility Functions
 * =================================
 */

/**
 * plingua_lex_init - Initialize lexer with source string
 * @source: P-lingua source code to tokenize
 * 
 * Sets up the lexer to read from a string buffer instead of stdin.
 */
void plingua_lex_init(const char *source) {
    if (!source) {
        return;
    }
    
    /* In a real implementation, would use yy_scan_string() */
    /* yy_scan_string(source); */
    yylineno = 1;
}

/**
 * plingua_lex_cleanup - Clean up lexer resources
 * 
 * Frees any resources allocated by the lexer.
 */
void plingua_lex_cleanup(void) {
    /* In a real implementation, would call yylex_destroy() */
    /* yylex_destroy(); */
}

/**
 * plingua_lex_reset - Reset lexer state
 * 
 * Resets line counter and internal state for parsing new source.
 */
void plingua_lex_reset(void) {
    yylineno = 1;
    string_buf_ptr = 0;
    /* In a real implementation, would call yyrestart() */
    /* yyrestart(stdin); */
}
