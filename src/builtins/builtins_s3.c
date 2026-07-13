#include "builtins_s3.h"

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ast/ast.h"
#include "io_backend/handle_input.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "utils/context.h"
#include "utils/myutils.h"

int exit_builtin(char **argv)
{
    int exit_code = get_context()->return_code;
    if (argv[1])
        exit_code = atoi(argv[1]);
    exit(exit_code);
}

static int init_env_var(void) // Initie l'environnement
{
    char cwd[4096];
    if (!getenv("PWD"))
    {
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            setenv("PWD", cwd, 1);
        }
        else
        {
            perror("42sh: init_env_var: getcwd failed");
            return 1;
        }
    }
    if (!getenv("OLDPWD"))
    {
        setenv("OLDPWD", getenv("PWD"), 1);
    }
    return 1;
}

static void update_pwd(char *new_path)
{
    char resolved_path[4096];
    if (realpath(new_path, resolved_path) == NULL)
    {
        setenv("PWD", new_path, 1);
    }
    else
    {
        setenv("PWD", resolved_path, 1);
    }
}

int cd_builtin(char **argv)
{
    if (getenv("PWD") == NULL || getenv("OLDPWD") == NULL)
    {
        init_env_var();
    }
    char *path = argv[1];
    char *oldpwd = getenv("PWD");

    if (!path)
    {
        path = getenv("HOME");
        if (!path)
        {
            fprintf(stderr, "42sh: cd: HOME not set\n");
            return 1;
        }
    }
    else if (strcmp(path, "-") == 0)
    {
        path = getenv("OLDPWD");
        if (!path)
        {
            fprintf(stderr, "42sh: cd: OLDPWD not set\n");
            return 1;
        }
        printf("%s\n", path);
    }

    if (chdir(path) == -1)
    {
        perror("42sh: cd: chdir failed");
        return 1;
    }

    setenv("OLDPWD", oldpwd, 1);
    update_pwd(path);
    return 0;
}

static int is_assignment_in(char *ch)
{
    if ((strchr(ch, '=') != NULL))
    {
        // verif si le nom de variable est alphanum_
        if (isdigit(ch[0]))
        {
            return 0;
        }
        size_t i = 0;
        while (ch[i] != '=')
        {
            if (ch[i] != '_' && !isalnum(ch[i]))
            {
                return 0;
            }
            i++;
        }
        return i; // retourne le rang du 1er '=' si good
    }
    return 0;
}

static void free_name_val(char *name, char *value, int alloc_flag)
{
    if (alloc_flag == 1)
    {
        free(name);
        free(value);
    }
}

int export_builtin(char **argv)
{
    if (!argv[1])
    {
        fprintf(stderr, "42sh: export: no variable given\n");
        return 1;
    }
    char *name = "a";
    char *value = "1";
    int alloc_flag = 0;
    for (int i = 1; argv[i]; i++)
    {
        int index = is_assignment_in(argv[i]);
        if (index != 0)
        {
            alloc_flag = 1;
            int j = 0;
            name = calloc(1, sizeof(char));
            while (j < index)
            {
                name[j] = argv[i][j];
                name = realloc(name, sizeof(char) * ((j++) + 2));
            }
            name[j++] = '\0';
            int k = 0;
            value = calloc(1, sizeof(char));
            while (argv[j])
            {
                value[k] = argv[i][j++];
                value = realloc(value, sizeof(char) * ((k++) + 2));
            }
            value[k] = '\0';
        }
        else
        {
            name = argv[i];
            value = retrieve_var_from_ctx(name, strlen(name));
        }
        if (value && setenv(name, value, 1) != -1)
        {
            free_name_val(name, value, alloc_flag);
        }
        else
        {
            free_name_val(name, value, alloc_flag);
            fprintf(stderr, "42sh: export: invalid format or setenv failed\n");
            return 1;
        }
    }
    return 0;
}

int break_builtin(char **argv)
{
    int nb = 0;
    if (argv[1])
        nb = atoi(argv[1]);
    return 0 - nb - 1;
}

int continue_builtin(char **argv)
{
    int nb = 0;
    if (argv[1])
        nb = atoi(argv[1]);
    return 300 + nb + 1;
}

static char *search_path(char *name)
{
    const char *env_path = getenv("PATH");
    if (!env_path)
    {
        perror("42sh: search_path: getenv failed");
        return NULL;
    }

    char *path = strdup(env_path);
    if (!path)
    {
        perror("42sh: search_path: strdup failed");
        return NULL;
    }

    char *token = strtok(path, ":");
    while (token)
    {
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", token, name);
        if (access(full_path, F_OK) == 0)
        {
            free(path);
            return strdup(full_path);
        }
        token = strtok(NULL, ":");
    }
    free(path);
    return NULL;
}

int dot_builtin(char **argv)
{
    if (!argv[1])
    {
        fprintf(stderr, "42sh: dot: missing filename\n");
        return 1;
    }
    int slash_flag = 0;
    for (size_t i = 0; argv[1][i]; i++)
    {
        if (argv[1][i] == '/')
        {
            slash_flag = 1;
            break;
        }
    }
    int argc = 1;
    char *res[20];
    res[0] = get_context()->args[0];
    if (slash_flag == 0)
    {
        res[1] = search_path(argv[1]);
        if (res[1] == NULL)
        {
            return 1;
        }
        res[2] = NULL;
        argc = 3;
    }
    else
    {
        size_t i = 1;
        for (; argv[i] != NULL; i++)
        {
            argc++;
            res[i] = argv[i];
        }
        res[i] = NULL;
    }
    while (res[argc])
        argc++;
    return sh_main(argc, res);
}

static int unset_variable(struct context *ctx, char *name)
{
    for (size_t i = 0; i < ctx->nb_variables; i++)
    {
        if (strcmp(ctx->variables[i].name, name) == 0)
        {
            unsetenv(ctx->variables[i].name);
            for (size_t j = i; j < ctx->nb_variables - 1; j++)
            {
                ctx->variables[j] = ctx->variables[j + 1];
            }
            ctx->nb_variables--;
            return 0;
        }
    }
    return 0;
}

static int unset_function(struct context *ctx, char *name)
{
    for (size_t i = 0; i < ctx->nb_functions; i++)
    {
        if (strcmp(ctx->functions[i].name, name) == 0)
        {
            ctx->functions[i].declared = 0;
            return 0;
        }
    }
    return 0;
}

int unset_builtin(char **argv)
{
    struct context *ctx = get_context();
    int unset_func_flag = 0;
    int unset_var_flag = 0;
    for (int i = 1; argv[i] && argv[i][0] == '-'; i++)
    {
        for (size_t j = 1; argv[i][j]; j++)
        {
            switch (argv[i][j])
            {
            case 'v':
                unset_var_flag = 1;
                break;
            case 'f':
                unset_func_flag = 1;
                break;
            default:
                fprintf(stderr, "42sh: unset: invalid flag %c\n", argv[i][j]);
                return 1;
            }
        }
    }

    if (unset_func_flag == 0 && unset_var_flag == 0)
        unset_var_flag = 1;

    for (int i = 1; argv[i]; i++)
    {
        if (argv[i][0] == '-')
            continue;
        if (unset_func_flag == 1)
        {
            if (unset_function(ctx, argv[i]) == 1)
                return 1;
        }
        if (unset_var_flag == 1)
        {
            if (unset_variable(ctx, argv[i]) == 1)
                return 1;
        }
    }
    return 0;
}

static int find_alias_index(struct context *ctx, char *name)
{
    for (size_t i = 0; i < ctx->nb_aliases; i++)
    {
        if (strcmp(ctx->aliases[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

static void add_alias(struct context *ctx, char *name, char *value)
{
    int index = find_alias_index(ctx, name);

    if (index != -1)
    {
        free(ctx->aliases[index].value);
        ctx->aliases[index].value = strdup(value);
    }
    else
    {
        ctx->nb_aliases++;
        ctx->aliases =
            realloc(ctx->aliases, ctx->nb_aliases * sizeof(struct alias));
        struct alias new = { .name = name, .value = value };
        ctx->aliases[ctx->nb_aliases - 1] = new;
    }
}

static int alphalias(const void *a, const void *b)
{
    struct alias *a1 = (struct alias *)a;
    struct alias *a2 = (struct alias *)b;
    return strcmp(a1->name, a2->name);
}

static void print_alias(struct context *ctx, char *name)
{
    int index = find_alias_index(ctx, name);
    if (index != -1)
    {
        printf("alias %s='%s'\n", ctx->aliases[index].name,
               ctx->aliases[index].value);
    }
    else
    {
        fprintf(stderr, "42sh: alias: %s not found\n", name);
    }
}

static void print_all_aliases(struct context *ctx)
{
    if (ctx->nb_aliases == 0)
        return;
    qsort(ctx->aliases, ctx->nb_aliases, sizeof(struct alias), alphalias);
    for (size_t i = 0; i < ctx->nb_aliases; i++)
    {
        printf("alias %s='%s'\n", ctx->aliases[i].name, ctx->aliases[i].value);
    }
}

int alias_builtin(char **argv)
{
    struct context *ctx = get_context();
    if (!argv[1])
    {
        print_all_aliases(ctx);
        return 0;
    }

    for (int i = 1; argv[i]; i++)
    {
        char *eq = strchr(argv[i], '=');
        if (eq)
        {
            *eq = '\0';
            add_alias(ctx, argv[i], eq + 1);
        }
        else
        {
            print_alias(ctx, argv[i]);
        }
    }
    return 0;
}

int unalias_builtin(char **argv)
{
    struct context *ctx = get_context();
    if (!argv[1])
    {
        fprintf(stderr, "42sh: unset: missing args\n");
        return 1;
    }

    for (int i = 1; argv[i]; i++)
    {
        int index = find_alias_index(ctx, argv[i]);
        if (index != -1)
        {
            for (size_t j = index; j < ctx->nb_aliases - 1; j++)
            {
                ctx->aliases[j] = ctx->aliases[j + 1];
            }
            ctx->nb_aliases--;
        }
        else
        {
            fprintf(stderr, "42sh: unalias: %s not found\n", argv[i]);
            return 1;
        }
    }
    return 0;
}
