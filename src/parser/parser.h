#ifndef PARSER_H
#define PARSER_H

#include "ast/ast.h"
#include "lexer/lexer.h"

enum parser_rule
{
    RULE_INPUT,
    RULE_LIST,
    RULE_AND_OR,
    RULE_PIPELINE,
    RULE_COMMAND,
    RULE_SIMPLE_COMMAND,
    RULE_SHELL_COMMAND,
    RULE_IF,
    RULE_ELSE,
    RULE_COMPLIST,
    RULE_WHILE,
    RULE_UNTIL,
    RULE_FOR,
    RULE_FUNCTION,
    RULE_SUBSHELL,
    RULE_CASE,
};

struct ast_base *parse_input(struct lexer *lexer);

struct ast_redir parse_redirection(struct lexer *lexer);

struct ast_base *parse_rule(enum parser_rule rule, struct lexer *lexer);

#endif /* PARSER_H */
