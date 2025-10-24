/*
 * P-lingua Grammar Parser for DTESN Kernel
 * =========================================
 * 
 * Yacc/Bison parser for P-lingua membrane computing rules.
 * Implements OEIS A000081-compliant membrane hierarchy parsing
 * and real-time rule compilation for the Echo.Kern.
 * 
 * P-lingua is a formal language for defining P-system membrane
 * computing rules with syntax: [obj]'membrane -> [result]'membrane
 */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "include/dtesn/psystem.h"

/* Parser state */
static dtesn_psystem_t *current_system = NULL;
static dtesn_psystem_membrane_t *current_membrane = NULL;
static dtesn_psystem_rule_t *current_rule = NULL;
static uint32_t rule_count = 0;
static uint32_t membrane_count = 0;

/* Error handling */
extern int yylex(void);
extern int yylineno;
extern char *yytext;

void yyerror(const char *s);

/* Forward declarations */
static int validate_oeis_a000081_compliance(uint32_t depth, uint32_t count);
static dtesn_psystem_membrane_t* create_membrane(const char *label, uint32_t depth);
static dtesn_psystem_rule_t* create_rule(int rule_type);
static void add_object_to_multiset(dtesn_psystem_multiset_t *multiset, 
                                   const char *symbol, uint32_t multiplicity);
%}

/* Token definitions */
%union {
    char *string;
    int integer;
    double real;
    struct {
        char *symbol;
        uint32_t multiplicity;
    } object;
}

%token <string> IDENTIFIER STRING_LITERAL
%token <integer> INTEGER
%token <real> REAL
%token ARROW LBRACKET RBRACKET LPAREN RPAREN
%token COMMA SEMICOLON COLON APOSTROPHE
%token MEMBRANE OBJECT RULE MULTISET PRIORITY
%token EVOLUTION COMMUNICATION DIVISION DISSOLUTION
%token DEF MODEL CALL END

%type <string> membrane_label object_symbol
%type <integer> multiplicity priority_level
%type <object> multiset_element

/* Operator precedence */
%left COMMA
%right ARROW

%%

/* Top-level program structure */
program:
    model_definition rules_section
    ;

model_definition:
    DEF MODEL IDENTIFIER LPAREN RPAREN SEMICOLON {
        /* Initialize P-system model */
        current_system = (dtesn_psystem_t *)malloc(sizeof(dtesn_psystem_t));
        if (current_system) {
            memset(current_system, 0, sizeof(dtesn_psystem_t));
            strncpy(current_system->name, $3, sizeof(current_system->name) - 1);
            current_system->max_depth = 8; /* Default OEIS A000081 depth */
        }
        free($3);
    }
    ;

rules_section:
    /* Empty */
    | rules_section rule_statement
    ;

rule_statement:
    evolution_rule
    | communication_rule
    | division_rule
    | dissolution_rule
    ;

/* Evolution rule: [u]'h -> [v]'h */
evolution_rule:
    LBRACKET multiset RBRACKET APOSTROPHE membrane_label 
    ARROW 
    LBRACKET multiset RBRACKET APOSTROPHE membrane_label 
    SEMICOLON {
        current_rule = create_rule(DTESN_PSYSTEM_RULE_EVOLUTION);
        if (current_rule) {
            strncpy(current_rule->source_membrane_label, $5, 
                   sizeof(current_rule->source_membrane_label) - 1);
            strncpy(current_rule->target_membrane_label, $11,
                   sizeof(current_rule->target_membrane_label) - 1);
            current_rule->priority = 1; /* Default priority */
            rule_count++;
        }
        free($5);
        free($11);
    }
    | LBRACKET multiset RBRACKET APOSTROPHE membrane_label 
    ARROW 
    LBRACKET multiset RBRACKET APOSTROPHE membrane_label
    PRIORITY INTEGER
    SEMICOLON {
        current_rule = create_rule(DTESN_PSYSTEM_RULE_EVOLUTION);
        if (current_rule) {
            strncpy(current_rule->source_membrane_label, $5,
                   sizeof(current_rule->source_membrane_label) - 1);
            strncpy(current_rule->target_membrane_label, $11,
                   sizeof(current_rule->target_membrane_label) - 1);
            current_rule->priority = $13;
            rule_count++;
        }
        free($5);
        free($11);
    }
    ;

/* Communication rule: [u]'h -> [v]'here [w]'out */
communication_rule:
    LBRACKET multiset RBRACKET APOSTROPHE membrane_label
    ARROW
    LBRACKET multiset RBRACKET APOSTROPHE IDENTIFIER
    LBRACKET multiset RBRACKET APOSTROPHE IDENTIFIER
    SEMICOLON {
        current_rule = create_rule(DTESN_PSYSTEM_RULE_COMMUNICATION);
        if (current_rule) {
            strncpy(current_rule->source_membrane_label, $5,
                   sizeof(current_rule->source_membrane_label) - 1);
            rule_count++;
        }
        free($5);
        free($11);
        free($16);
    }
    ;

/* Division rule: [a]'h -> [b]'h [c]'h */
division_rule:
    LBRACKET multiset RBRACKET APOSTROPHE membrane_label
    ARROW
    LBRACKET multiset RBRACKET APOSTROPHE membrane_label
    LBRACKET multiset RBRACKET APOSTROPHE membrane_label
    SEMICOLON {
        current_rule = create_rule(DTESN_PSYSTEM_RULE_DIVISION);
        if (current_rule) {
            strncpy(current_rule->source_membrane_label, $5,
                   sizeof(current_rule->source_membrane_label) - 1);
            current_rule->creates_membranes = 1; /* Division creates new membrane */
            rule_count++;
            
            /* Validate OEIS A000081 compliance for division */
            uint32_t depth = 0; /* Would need to calculate actual depth */
            if (!validate_oeis_a000081_compliance(depth, membrane_count + 1)) {
                yyerror("Division would violate OEIS A000081 enumeration");
            }
        }
        free($5);
        free($11);
        free($16);
    }
    ;

/* Dissolution rule: [a]'h -> [] */
dissolution_rule:
    LBRACKET multiset RBRACKET APOSTROPHE membrane_label
    ARROW
    LBRACKET RBRACKET
    SEMICOLON {
        current_rule = create_rule(DTESN_PSYSTEM_RULE_DISSOLUTION);
        if (current_rule) {
            strncpy(current_rule->source_membrane_label, $5,
                   sizeof(current_rule->source_membrane_label) - 1);
            current_rule->dissolves_membrane = 1;
            rule_count++;
        }
        free($5);
    }
    ;

/* Multiset: collection of objects with multiplicities */
multiset:
    /* Empty multiset */
    | multiset_elements
    ;

multiset_elements:
    multiset_element {
        /* Add object to current multiset */
        if (current_rule && current_rule->lhs_multiset) {
            add_object_to_multiset(current_rule->lhs_multiset, 
                                  $1.symbol, $1.multiplicity);
        }
        free($1.symbol);
    }
    | multiset_elements COMMA multiset_element {
        if (current_rule && current_rule->lhs_multiset) {
            add_object_to_multiset(current_rule->lhs_multiset,
                                  $3.symbol, $3.multiplicity);
        }
        free($3.symbol);
    }
    ;

multiset_element:
    object_symbol {
        $$.symbol = $1;
        $$.multiplicity = 1;
    }
    | multiplicity object_symbol {
        $$.symbol = $2;
        $$.multiplicity = $1;
    }
    ;

object_symbol:
    IDENTIFIER { $$ = $1; }
    ;

membrane_label:
    IDENTIFIER { $$ = $1; }
    ;

multiplicity:
    INTEGER { $$ = $1; }
    ;

priority_level:
    INTEGER { $$ = $1; }
    ;

%%

/* Error handling */
void yyerror(const char *s) {
    fprintf(stderr, "P-lingua parse error at line %d: %s near '%s'\n",
            yylineno, s, yytext);
}

/**
 * validate_oeis_a000081_compliance - Validate membrane hierarchy
 * 
 * Ensures membrane structure follows OEIS A000081 enumeration:
 * 1, 1, 2, 4, 9, 20, 48, 115, 286, 719, ...
 */
static int validate_oeis_a000081_compliance(uint32_t depth, uint32_t count) {
    /* OEIS A000081 sequence */
    static const uint32_t a000081[] = {
        1, 1, 1, 2, 4, 9, 20, 48, 115, 286, 719, 1842, 4766, 12486
    };
    
    if (depth >= sizeof(a000081) / sizeof(a000081[0])) {
        return 0; /* Depth too large */
    }
    
    return count <= a000081[depth];
}

/**
 * create_membrane - Create new membrane structure
 */
static dtesn_psystem_membrane_t* create_membrane(const char *label, uint32_t depth) {
    if (!current_system || membrane_count >= DTESN_PSYSTEM_MAX_MEMBRANES) {
        return NULL;
    }
    
    /* Validate OEIS A000081 compliance */
    if (!validate_oeis_a000081_compliance(depth, membrane_count + 1)) {
        yyerror("Membrane creation would violate OEIS A000081 enumeration");
        return NULL;
    }
    
    dtesn_psystem_membrane_t *membrane = 
        &current_system->membranes_storage[membrane_count];
    
    memset(membrane, 0, sizeof(dtesn_psystem_membrane_t));
    membrane->membrane_id = membrane_count;
    strncpy(membrane->label, label, sizeof(membrane->label) - 1);
    membrane->depth_level = depth;
    membrane->is_dissolved = 0;
    
    current_system->membranes[membrane_count] = membrane;
    membrane_count++;
    current_system->membrane_count = membrane_count;
    
    return membrane;
}

/**
 * create_rule - Create new P-system rule
 */
static dtesn_psystem_rule_t* create_rule(int rule_type) {
    if (!current_system || rule_count >= DTESN_PSYSTEM_MAX_RULES) {
        return NULL;
    }
    
    dtesn_psystem_rule_t *rule = &current_system->rules_storage[rule_count];
    
    memset(rule, 0, sizeof(dtesn_psystem_rule_t));
    rule->rule_id = rule_count;
    rule->rule_type = rule_type;
    rule->priority = 1;
    rule->is_active = 1;
    
    /* Allocate multisets */
    rule->lhs_multiset = (dtesn_psystem_multiset_t *)
        malloc(sizeof(dtesn_psystem_multiset_t));
    rule->rhs_multiset = (dtesn_psystem_multiset_t *)
        malloc(sizeof(dtesn_psystem_multiset_t));
    
    if (rule->lhs_multiset) {
        memset(rule->lhs_multiset, 0, sizeof(dtesn_psystem_multiset_t));
    }
    if (rule->rhs_multiset) {
        memset(rule->rhs_multiset, 0, sizeof(dtesn_psystem_multiset_t));
    }
    
    current_system->rules[rule_count] = rule;
    
    return rule;
}

/**
 * add_object_to_multiset - Add object with multiplicity to multiset
 */
static void add_object_to_multiset(dtesn_psystem_multiset_t *multiset,
                                   const char *symbol, uint32_t multiplicity) {
    if (!multiset || !symbol) {
        return;
    }
    
    if (multiset->num_symbols >= DTESN_PSYSTEM_MAX_OBJECTS) {
        return;
    }
    
    /* Check if symbol already exists */
    for (uint32_t i = 0; i < multiset->num_symbols; i++) {
        if (strcmp(multiset->symbols[i], symbol) == 0) {
            multiset->multiplicities[i] += multiplicity;
            return;
        }
    }
    
    /* Add new symbol */
    uint32_t idx = multiset->num_symbols;
    strncpy(multiset->symbols[idx], symbol, 
           sizeof(multiset->symbols[idx]) - 1);
    multiset->multiplicities[idx] = multiplicity;
    multiset->num_symbols++;
}

/**
 * plingua_parse_system - Main parsing entry point
 * 
 * Parse P-lingua source and create DTESN P-system structure.
 */
int plingua_parse_system(const char *source, dtesn_psystem_t **system_out) {
    if (!source || !system_out) {
        return -1;
    }
    
    /* Reset parser state */
    current_system = NULL;
    current_membrane = NULL;
    current_rule = NULL;
    rule_count = 0;
    membrane_count = 0;
    
    /* Parse would be initiated here by calling yyparse() */
    /* In real implementation, would set up input buffer for lexer */
    
    if (current_system) {
        *system_out = current_system;
        return 0;
    }
    
    return -1;
}
