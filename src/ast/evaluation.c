#include <builtins/builtins_s1_2.h>
#include <builtins/builtins_s3.h>
#include <err.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <lexer/lexer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utils/context.h>
#include <utils/expansion.h>
#include <utils/myutils.h>

#include "ast.h"

typedef int (*type_builtin)(char **args);
static char *builtins_names[] = { "echo",  "true",     "false", "cd",
                                  ".",     "export",   "exit",  "unset",
                                  "break", "continue", "alias", "unalias",
                                  NULL };

static type_builtin builtins_func[] = {
    echo_builtin,  true_builtin,     false_builtin, cd_builtin,
    dot_builtin,   export_builtin,   exit_builtin,  unset_builtin,
    break_builtin, continue_builtin, alias_builtin, unalias_builtin
};

struct redirect
{
    int fd_left;
    int fd_right;
    int is_file;
};
typedef int (*run_type)(struct ast_base *ast);

void save_redir(struct ast_redir redir, struct redirect **saved, size_t i)
{
    if ((*saved)[i].is_file)
        close((*saved)[i].fd_right);
    if (redir.redir_type == REDIR_LEFT_STD
        || redir.redir_type == REDIR_RIGHT_STD)
    {
        (*saved)[i].fd_right = atoi(redir.filename);
        (*saved)[i].is_file = 0;
    }
    else
    {
        if (redir.redir_type == REDIR_RIGHT_APPEND)
        {
            (*saved[i]).fd_right =
                open(redir.filename, O_CREAT | O_APPEND | O_WRONLY, 0644);
        }
        else if (redir.redir_type == REDIR_RIGHT)
        {
            (*saved)[i].fd_right =
                open(redir.filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
        }
        else
        {
            (*saved)[i].fd_right = open(redir.filename, O_RDONLY, 0644);
        }

        (*saved)[i].is_file = 1;
    }
}

void check_saved(struct ast_redir redir, struct redirect **saved, size_t *size)
{
    int changed = 0;
    for (size_t i = 0; i < *size; i++)
    {
        if (redir.io_number == saved[i]->fd_left)
        {
            save_redir(redir, saved, i);
            changed = 1;
            break;
        }
    }
    if (!changed)
    {
        (*size)++;
        *saved = realloc(*saved, *size * sizeof(struct redirect));
        (*saved)[*size - 1].is_file = 0;
        (*saved)[*size - 1].fd_left = redir.io_number;
        save_redir(redir, saved, *size - 1);
    }
}

static int run_case(struct ast_base *node)
{
    struct ast_case *caseoh = (struct ast_case *)node;
    struct context *ctx = get_context();

    char **temp_list = calloc(2, sizeof(char *));
    temp_list[0] = caseoh->word;
    char **temp_list_expanded = scan_expand_variable(temp_list);
    free(temp_list);
    caseoh->word = temp_list_expanded[0];
    free(temp_list_expanded);

    for (size_t i = 0; i < caseoh->nb_cases; i++)
    {
        size_t j = 0;
        char **case_expended = scan_expand_variable(caseoh->cases[i].values);
        while (case_expended[j])
        {
            if (fnmatch(case_expended[j], caseoh->word, 0) == 0)
            {
                free(case_expended);
                return (ctx->return_code = run_ast(caseoh->cases[i].body));
            }
            j++;
        }
        free(case_expended);
    }
    return 0;
}

static int run_subshell(struct ast_base *node)
{
    struct ast_subshell *shell = (struct ast_subshell *)node;
    struct context *ctx = get_context();
    if (fork() == 0)
    {
        int res = run_ast(shell->content);
        exit(res);
    }
    else
    {
        int status;
        wait(&status);
        return (ctx->return_code = WEXITSTATUS(status));
    }
}

static int run_func(struct ast_base *node)
{
    struct ast_func *func = (struct ast_func *)node;
    struct context *ctx = get_context();
    for (size_t i = 0; i < ctx->nb_functions; i++)
    {
        if (strcmp(ctx->functions[i].name, func->name) == 0)
        {
            ctx->functions[i].declared = 1;
            return (ctx->return_code = 0);
        }
    }

    return (ctx->return_code = 1);
}

static int run_forlist(struct ast_base *node)
{
    (void)node;
    return 0;
}

static int run_redir(struct ast_base *node)
{
    struct ast_redir_handle *handle = (struct ast_redir_handle *)node;
    struct context *ctx = get_context();
    size_t size = 0;
    struct redirect *saved_redir = NULL;
    if (handle->dict_size > 0)
    {
        scan_expand_assignments(handle);
        for (int n = 0; n < handle->dict_size; n++)
        {
            add_var_to_context(handle->assignment_dict[n].key,
                               handle->assignment_dict[n].value);
        }
    }
    scan_expand_redirs(handle);
    for (int i = 0; i < handle->size; i++)
    {
        check_saved(handle->redir[i], &saved_redir, &size);
    }
    for (size_t j = 0; j < size; j++)
    {
        int std = saved_redir[j].fd_left;
        int save_left = dup(std);
        if (save_left == -1)
            errx(1, "evaluation: Unable to dup fd");
        if (dup2(saved_redir[j].fd_right, saved_redir[j].fd_left) == -1)
            errx(1, "evaluation: Unable to dup fd");
        saved_redir[j].fd_right = save_left;
    }
    int res = run_ast(handle->child);
    for (size_t j = 0; j < size; j++)
    {
        dup2(saved_redir[j].fd_right, saved_redir[j].fd_left);
        close(saved_redir[j].fd_right);
    }
    free(saved_redir);
    return (ctx->return_code = res);
}

static int run_neg(struct ast_base *node)
{
    struct ast_neg *neg = (struct ast_neg *)node;
    struct context *ctx = get_context();
    return (ctx->return_code = !run_ast(neg->child));
}

static int run_and(struct ast_base *node)
{
    struct ast_cond *and = (struct ast_cond *)node;
    struct context *ctx = get_context();
    return (ctx->return_code = !(!run_ast(and->left) && !run_ast(and->right)));
}
static int run_or(struct ast_base *node)
{
    struct ast_cond * or = (struct ast_cond *)node;
    struct context *ctx = get_context();
    return (ctx->return_code = !(!run_ast(or->left) || !run_ast(or->right)));
}

static int run_pipeline(struct ast_base *node)
{
    struct ast_pipeline *pipe_node = (struct ast_pipeline *)node;
    struct context *ctx = get_context();
    int save_in = dup(STDIN_FILENO);
    int save_out = dup(STDOUT_FILENO);
    int status = 0;
    int pipefd[2];
    int res = 0;
    if (pipe(pipefd) == -1)
        errx(1, "evaluate: unable to create pipe");
    if (fork() == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        res = run_ast(pipe_node->left);
        exit(res);
    }
    else
    {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        res = run_ast(pipe_node->right);
        dup2(save_out, STDOUT_FILENO);
        dup2(save_in, STDIN_FILENO);
        close(save_out);
        close(save_in);
        return (ctx->return_code = WEXITSTATUS(status));
    }

    return (ctx->return_code = 0);
}

static int is_interrupt(int *level)
{
    struct context *ctx = get_context();
    if (*level < 0)
    {
        (*level)++;
        if (*level == -1 || ctx->loop == 0)
        {
            return 1; // last break
        }
        return 2; // more break to go
    }
    if (*level > 300)
    {
        (*level)--;
        if (*level == 301 || ctx->loop == 1)
        {
            return 3; // last continue
        }
        return 4; // more continue to go
    }
    return 0; // not an interrupt
}

static int run_loop(struct ast_base *node)
{
    struct ast_loop *loop = (struct ast_loop *)node;
    struct context *ctx = get_context();
    int res = 0;
    int inter = 0;
    if (node->type == AST_UNTIL)
    {
        while (run_ast(loop->condition))
        {
            ctx->loop++;
            res = run_ast(loop->body);
            inter = is_interrupt(&res);
            ctx->loop--;
            if (inter == 1)
                return (ctx->return_code = 0);
            if (inter == 2 || inter == 4)
                return res;
        }
    }
    else if (node->type == AST_WHILE)
    {
        while (!run_ast(loop->condition)) // True and False are inverted because
                                          // we use shell boolean
        {
            ctx->loop++;
            res = run_ast(loop->body);
            inter = is_interrupt(&res);
            ctx->loop--;
            if (inter == 1)
                return (ctx->return_code = 0);
            if (inter == 2 || inter == 4)
                return res;
        }
    }
    else
    {
        struct ast_for_list *forlist = (struct ast_for_list *)loop->condition;
        size_t i = 0;
        char **list = forlist->list;
        while (list[i])
        {
            ctx->loop++;
            add_var_to_context(forlist->word, list[i++]);
            res = run_ast(loop->body);
            inter = is_interrupt(&res);
            ctx->loop--;
            if (inter == 1)
                return (ctx->return_code = 0);
            if (inter == 2 || inter == 4)
                return res;
        }
    }
    return (inter != 0 ? (ctx->return_code = 0) : (ctx->return_code = res));
}

static int run_list(struct ast_base *node)
{
    struct ast_list *list_node = (struct ast_list *)node;
    struct context *ctx = get_context();
    size_t i = 0;
    int res = 1;
    while (list_node->children[i])
    {
        res = run_ast(list_node->children[i++]);
        if (res >= 300 || res < 0)
            return res;
    }
    return (ctx->return_code = res);
}

static int run_if(struct ast_base *node)
{
    struct ast_if *if_node = (struct ast_if *)node;
    struct context *ctx = get_context();
    if (!run_ast(if_node->condition))
    {
        return (ctx->return_code = run_ast(if_node->then_body));
    }
    else if (if_node->else_body)
    {
        return (ctx->return_code = run_ast(if_node->else_body));
    }
    return (ctx->return_code = 0);
}

static int run_cmd(struct ast_base *node)
{
    struct ast_simple_command *command = (struct ast_simple_command *)node;
    struct context *ctx = get_context();
    int res = 0;
    char **expanded_args = scan_expand_variable(command->args);
    size_t i = 0;
    for (size_t n = 0; n < ctx->nb_functions; n++)
    {
        if (ctx->functions[n].declared
            && strcmp(ctx->functions[n].name, expanded_args[0]) == 0)
        {
            free(expanded_args);
            return run_ast(ctx->functions[n].func);
        }
    }
    while (builtins_names[i])
    {
        if (strcmp(expanded_args[0], builtins_names[i]) == 0)
        {
            res = builtins_func[i](expanded_args);
            break;
        }
        i++;
    }
    if (!builtins_names[i])
        res = args_builtin(expanded_args);
    fflush(stdout);
    free(expanded_args);
    return (ctx->return_code = res);
}

int run_ast(struct ast_base *ast)
{
    if (!ast)
        return 0;
    struct context *ctx = get_context();
    static const run_type functions[] = {
#define XAST_F(Enum, Name) [AST_##Enum] = run_##Name,
        XAST(XAST_F)
#undef XAST_F
    };
    return (ctx->return_code = (*functions[ast->type])(ast));
}
