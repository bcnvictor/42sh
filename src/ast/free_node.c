#include <stdlib.h>

#include "ast.h"

typedef void (*free_type)(struct ast_base *ast);

static void free_case(struct ast_base *node)
{
    struct ast_case *case_node = (struct ast_case *)node;
    for (size_t i = 0; i < case_node->nb_cases; i++)
    {
        free_ast(case_node->cases[i].body);
        free(case_node->cases[i].values);
    }
    free(case_node->cases);
    free(node);
}

static void free_func(struct ast_base *node)
{
    struct ast_func *func = (struct ast_func *)node;
    free(func);
}

static void free_subshell(struct ast_base *node)
{
    struct ast_subshell *shell = (struct ast_subshell *)node;
    free_ast(shell->content);
    free(shell);
}
static void free_redir(struct ast_base *node)
{
    struct ast_redir_handle *redir = (struct ast_redir_handle *)node;
    free_ast(redir->child);
    free(redir->redir);
    if (redir->assignment_dict)
    {
        free(redir->assignment_dict);
    }
    free(redir);
}

static void free_neg(struct ast_base *node)
{
    struct ast_neg *neg = (struct ast_neg *)node;
    free_ast(neg->child);
    free(neg);
}

static void free_and(struct ast_base *node)
{
    struct ast_cond *and = (struct ast_cond *)node;
    free_ast(and->left);
    free_ast(and->right);
    free(and);
}

static void free_or(struct ast_base *node)
{
    struct ast_cond * or = (struct ast_cond *)node;
    free_ast(or->left);
    free_ast(or->right);
    free(or);
}

static void free_pipeline(struct ast_base *node)
{
    struct ast_pipeline *pipe_node = (struct ast_pipeline *)node;
    free_ast(pipe_node->left);
    free_ast(pipe_node->right);
    free(pipe_node);
}

static void free_forlist(struct ast_base *node)
{
    struct ast_for_list *forlist = (struct ast_for_list *)node;
    free(forlist->list);
    free(forlist);
}

static void free_loop(struct ast_base *node)
{
    struct ast_loop *loop = (struct ast_loop *)node;
    free_ast(loop->condition);
    free_ast(loop->body);
    free(loop);
}

static void free_list(struct ast_base *node)
{
    struct ast_list *list_node = (struct ast_list *)node;
    size_t i = 0;
    while (list_node->children[i])
    {
        free_ast(list_node->children[i++]);
    }
    free(list_node->children);
    free(list_node);
}

static void free_if(struct ast_base *node)
{
    struct ast_if *if_node = (struct ast_if *)node;
    free_ast(if_node->condition);
    free_ast(if_node->then_body);
    free_ast(if_node->else_body);
    free(if_node);
}

static void free_cmd(struct ast_base *node)
{
    struct ast_simple_command *command = (struct ast_simple_command *)node;
    free(command->args);
    free(command);
}

void free_ast(struct ast_base *ast)
{
    if (!ast)
        return;
    static const free_type functions[] = {
#define XAST_F(Enum, Name) [AST_##Enum] = free_##Name,
        XAST(XAST_F)
#undef XAST_F
    };
    (*functions[ast->type])(ast);
}
