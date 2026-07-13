#ifndef AST_H
#define AST_H

#define XAST(F)                                                                \
    F(CMD, cmd)                                                                \
    F(IF, if)                                                                  \
    F(LIST, list)                                                              \
    F(PIPELINE, pipeline)                                                      \
    F(WHILE, loop)                                                             \
    F(UNTIL, loop)                                                             \
    F(AND, and)                                                                \
    F(OR, or)                                                                  \
    F(NEG, neg)                                                                \
    F(REDIR, redir)                                                            \
    F(FOR, loop)                                                               \
    F(FORLIST, forlist)                                                        \
    F(FUNC, func)                                                              \
    F(SUBSHELL, subshell)                                                      \
    F(CASE, case)

#include <stdlib.h>

#include "lexer/lexer.h"
#include "utils/myutils.h"

enum node_type
{
#define XAST_F(Enum, Name) AST_##Enum,
    XAST(XAST_F)
#undef XAST_F
};

struct dict
{
    char *key;
    char *value;
};

struct ast_node
{
    enum node_type type;
    char *value;
    struct ast_node *left;
    struct ast_node *right;
};

struct ast_base
{
    enum node_type type;
};

struct ast_if
{
    struct ast_base base;
    struct ast_base *condition;
    struct ast_base *then_body;
    struct ast_base *else_body;
};

struct ast_list
{
    struct ast_base base;
    struct ast_base **children;
};

struct ast_simple_command
{
    struct ast_base base;
    char **args;
};

struct ast_pipeline
{
    struct ast_base base;
    int negation;
    struct ast_base *left;
    struct ast_base *right;
};

struct ast_loop
{
    struct ast_base base;
    struct ast_base *condition;
    struct ast_base *body;
};

struct ast_for_list
{
    struct ast_base base;
    char *word;
    char **list;
};

struct ast_cond
{
    struct ast_base base;
    struct ast_base *left;
    struct ast_base *right;
};

struct ast_neg
{
    struct ast_base base;
    struct ast_base *child;
};

struct ast_redir_handle
{
    struct ast_base base;
    struct ast_base *child;
    struct ast_redir *redir;
    struct dict *assignment_dict;
    int size;
    int dict_size;
};

struct ast_redir
{
    int io_number;
    enum token_type redir_type;
    char *filename;
};

struct ast_func
{
    struct ast_base base;
    char *name;
    struct ast_base *start;
};

struct ast_subshell
{
    struct ast_base base;
    struct ast_base *content;
};

struct case_handler
{
    char **values;
    struct ast_base *body;
};

struct ast_case
{
    struct ast_base base;
    char *word;
    struct case_handler *cases;
    size_t nb_cases;
};

void ast_pretty_print(struct ast_base *ast);

int evaluate_ast(struct ast_base *node);

int run_ast(struct ast_base *ast);
void free_ast(struct ast_base *ast);
void create_dot_file(struct ast_base *ast, char *filename);
const char *ast_repr(struct ast_base *node);

#endif /* !AST_H */
