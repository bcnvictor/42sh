#ifndef CONTEXT_H
#define CONTEXT_H

#include <stddef.h>
#include <stdio.h>

#include "expansion.h"

struct context
{
    int arg_start;
    int return_code;
    char **args;
    int nb_args;
    char *buffer;
    char **allocated_strings;
    size_t nb_strings;
    struct var *variables;
    struct function *functions;
    struct alias *aliases;
    size_t nb_aliases;
    size_t nb_functions;
    size_t nb_variables;
    int quoted;
    int escaped;
    int contains_variable;
    FILE *fd;
    int curpid;
    int loop;
    int seed;
};

struct alias
{
    char *name;
    char *value;
};

struct function
{
    int declared;
    char *name;
    struct ast_base *func;
};

struct var
{
    char *name;
    char *value;
};

struct context *init_context(void);
struct context *get_context(void);
void add_string_to_context(char *word);
void add_var_to_context(char *name, char *value);
void add_func_to_context(char *name, struct ast_base *func, int declaration);
char *retrieve_var_from_ctx(char *name, int length);
void clear_context(struct context *ctx);
void free_context(struct context *ctx);
char *get_variable(char *name, int size, struct expansion_handle *exp);
int sh_main(int argc, char *argv[]);

#endif /* !CONTEXT_H */
