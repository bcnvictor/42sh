#include "context.h"

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ast/ast.h"
#include "expansion.h"

static struct context *ctx = NULL;

struct context *get_context(void)
{
    return ctx;
}

struct context *init_context(void)
{
    if (!ctx)
    {
        ctx = calloc(1, sizeof(struct context));
        ctx->curpid = getpid();
        ctx->seed = 1;
        return ctx;
    }
    errx(1, "context: context already exist");
}

void add_string_to_context(char *s)
{
    struct context *ctx = get_context();
    ctx->nb_strings++;
    ctx->allocated_strings =
        realloc(ctx->allocated_strings, ctx->nb_strings * sizeof(char *));
    ctx->allocated_strings[ctx->nb_strings - 1] = s;
}

void add_func_to_context(char *name, struct ast_base *func, int declaration)
{
    for (size_t i = 0; i < ctx->nb_functions; i++)
    {
        if (strcmp(ctx->functions[i].name, name) == 0)
        {
            if (ctx->functions[i].func != func)
                free_ast(ctx->functions[i].func);
            ctx->functions[i].declared = declaration;
            ctx->functions[i].func = func;
            return;
        }
    }
    ctx->nb_functions++;
    ctx->functions =
        realloc(ctx->functions, ctx->nb_functions * sizeof(struct function));
    struct function new = { .declared = declaration,
                            .name = name,
                            .func = func };
    ctx->functions[ctx->nb_functions - 1] = new;
}

void add_var_to_context(char *name, char *value)
{
    struct context *ctx = get_context();
    for (size_t i = 0; i < ctx->nb_variables; i++)
    {
        if (strcmp(ctx->variables[i].name, name) == 0)
        {
            ctx->variables[i].value = value;
            return;
        }
    }
    ctx->nb_variables++;
    ctx->variables =
        realloc(ctx->variables, ctx->nb_variables * sizeof(struct var));
    struct var new = { .name = name, .value = value };
    ctx->variables[ctx->nb_variables - 1] = new;
}

// free all token strings allocated during lexing
void clear_context(struct context *ctx)
{
    size_t counter = 0;
    while (counter < ctx->nb_strings)
    {
        free(ctx->allocated_strings[counter++]);
    }
    ctx->nb_strings = 0;
    free(ctx->allocated_strings);
}

static void free_functions(void)
{
    for (size_t i = 0; i < ctx->nb_functions; i++)
    {
        free_ast(ctx->functions[i].func);
    }
    free(ctx->functions);
}

void free_all_alias(struct context *ctx)
{
    free(ctx->aliases);
}

void free_context(struct context *ctx)
{
    if (ctx)
    {
        if (ctx->buffer)
        {
            free(ctx->buffer);
        }
        if (ctx->variables)
        {
            free(ctx->variables);
        }
        if (ctx->allocated_strings)
        {
            clear_context(ctx);
        }
        if (ctx->nb_functions > 0)
        {
            free_functions();
        }
        if (ctx->nb_aliases > 0)
        {
            free_all_alias(ctx);
        }
        free(ctx);
    }
}

// $*
static char *concat_all_args(struct expansion_handle *exp)
{
    if (!ctx->args[ctx->arg_start])
        return "";
    size_t i = ctx->arg_start;
    char *res = NULL;
    size_t s = 0;
    while (ctx->args[i])
    {
        res = realloc(res, s + strlen(ctx->args[i]) + 2);
        strcpy(res + s, ctx->args[i]);
        s += strlen(ctx->args[i++]);
        res[s++] = ' ';
    }
    res[--s] = 0;
    exp->variable_index += s;
    add_string_to_context(res);
    return res;
}

// $?
static int get_return_code(void)
{
    return ctx->return_code;
}

// $$
static int get_pid(void)
{
    return ctx->curpid;
}
// $1...$n
static char *get_arg(int n)
{
    if (n < 1)
        errx(1, "n must be greater than 1");
    if (ctx->arg_start + n - 1 >= ctx->nb_args)
        return "";
    return ctx->args[ctx->arg_start + n - 1];
}
// $#
static int get_nb_args(void)
{
    int nb = 0;
    while (ctx->args[ctx->arg_start + nb])
    {
        nb++;
    }
    return nb - 1;
}
// $RANDOM
static int get_random(void)
{
    srand(ctx->seed);
    ctx->seed++;
    return rand() % 32767;
}
// $UID
static int get_uid(void)
{
    return getuid();
}

char *retrieve_var_from_ctx(char *name, int length)
{
    struct context *ctx = get_context();
    char *cur;
    for (size_t i = 0; i < ctx->nb_variables; i++)
    {
        cur = ctx->variables[i].name;
        if (strlen(cur) == (size_t)length && strncmp(name, cur, length) == 0)
        {
            return ctx->variables[i].value;
        }
    }
    return "";
}

static char *handle_custom_var(char *name, int *size, int length)
{
    char *res;
    int code;
    int s;
    if ('0' <= name[0] && name[0] <= '9')
    {
        res = get_arg(name[0] - '0');
        *size = strlen(res);
        return res;
    }
    if (strncmp(name, "RANDOM", length) == 0)
    {
        code = get_random();
        s = snprintf(NULL, 0, "%d", code) + 1;
        res = calloc(s, sizeof(char));
        add_string_to_context(res);
        sprintf(res, "%d", code);
        *size = s;
        return res;
    }
    if (strncmp(name, "UID", length) == 0)
    {
        code = get_uid();
        s = snprintf(NULL, 0, "%d", code) + 1;
        res = calloc(s, sizeof(char));
        add_string_to_context(res);
        sprintf(res, "%d", code);
        *size = s;
        return res;
    }
    if (strncmp(name, "PWD", length) == 0)
    {
        res = getenv("PWD");
        *size = strlen(res);
        return res;
    }
    if (strncmp(name, "OLDPWD", length) == 0)
    {
        res = getenv("OLDPWD");
        *size = strlen(res);
        return res;
    }
    res = retrieve_var_from_ctx(name, length);
    *size = strlen(res);
    return res;
}

// $@
static void handle_all_args(struct expansion_handle *exp)
{
    if (!ctx->args[ctx->arg_start])
        return;
    size_t i = ctx->arg_start;
    int index = exp->variable_index;
    exp->variable_index += strlen(ctx->args[i]);
    if (exp->arg_size < exp->variable_index + 1)
    {
        exp->argv[exp->arg_index] =
            realloc(exp->argv[exp->arg_index], exp->variable_index + 1);
        exp->arg_size = exp->variable_index + 1;
    }
    strcpy(exp->argv[exp->arg_index] + index, ctx->args[i++]);
    exp->arg_index++;
    while (ctx->args[i])
    {
        exp->argv = realloc(exp->argv, (exp->arg_index + 1) * sizeof(char *));
        exp->argv[exp->arg_index] =
            calloc(strlen(ctx->args[i]) + 2, sizeof(char));
        exp->arg_size = strlen(ctx->args[i]) + 2;
        strcpy(exp->argv[exp->arg_index], ctx->args[i]);
        exp->variable_index = strlen(ctx->args[i++]);
        exp->arg_index++;
    }
    exp->argv[--exp->arg_index][exp->variable_index] = 0;
}

char *get_variable(char *name, int size, struct expansion_handle *exp)
{
    int code;
    char *res = NULL;
    int s = 0;
    switch (name[0])
    {
    case '@':
        handle_all_args(exp);
        return "";
    case '*':
        return concat_all_args(exp);
    case '?':
        code = get_return_code();
        s = snprintf(NULL, 0, "%d", code);
        res = calloc(s + 1, sizeof(char));
        sprintf(res, "%d", code);
        add_string_to_context(res);
        break;
    case '$':
        code = get_pid();
        s = snprintf(NULL, 0, "%d", code);
        res = calloc(s + 1, sizeof(char));
        sprintf(res, "%d", code);
        add_string_to_context(res);
        break;
    case '#':
        code = get_nb_args();
        s = snprintf(NULL, 0, "%d", code);
        res = calloc(s + 1, sizeof(char));
        sprintf(res, "%d", code);
        add_string_to_context(res);
        break;
    default:
        res = handle_custom_var(name, &s, size);
        break;
    }
    exp->variable_index += s;
    return res;
}
