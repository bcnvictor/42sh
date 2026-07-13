#ifndef EXPANSION_H
#define EXPANSION_H

#include "ast/ast.h"

struct expansion_handle
{
    int argc;
    char **argv;
    int arg_index;
    int read_index; // cursor for reading input strings
    int arg_size;
    int variable_index; // cursor for writing output strings
};

char **scan_expand_variable(char **argv);

void scan_expand_assignments(struct ast_redir_handle *hredir);

void scan_expand_redirs(struct ast_redir_handle *hredir);

void expand_var(struct expansion_handle *exp, char *var, int var_length);

char *expand_cmd_sub(char *subcmd);

void scan_input(char *input, struct expansion_handle *h);

void command_substitution(char *input, struct expansion_handle *h,
                          char *current, int *max_size_expanded);

#endif /* ! EXPANSION_H */
