#include "parser.h"

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast/ast.h"
#include "lexer/lexer.h"
#include "utils/context.h"
#include "utils/myutils.h"

static int first_command(struct token tok)
{
    return tok.type == TOKEN_WORD || is_redir(tok.type) || tok.type == TOKEN_IF
        || tok.type == TOKEN_WHILE || tok.type == TOKEN_UNTIL
        || tok.type == TOKEN_FOR || tok.type == TOKEN_LEFT_BRA
        || tok.type == TOKEN_LEFT_PAR || tok.type == TOKEN_CASE;
}

static void skip_newlines(struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);
    while (tok.type == TOKEN_NEWLINE)
    {
        lexer_pop(lexer);
        tok = lexer_peek(lexer);
    }
}

static int check_token(struct lexer *lexer, enum token_type expected)
{
    struct token tok = lexer_peek(lexer);
    if (tok.type != expected)
    {
        return 0;
    }
    return 1;
}

static int is_assignment(struct token tok)
{
    if (tok.type == TOKEN_WORD && (strchr(tok.word, '=') != NULL))
    {
        // verif si le nom de variable est alphanum_
        if (isdigit(tok.word[0]))
        {
            return 0;
        }
        size_t i = 0;
        while (tok.word[i] != '=')
        {
            if (tok.word[i] != '_' && !isalnum(tok.word[i]))
            {
                return 0;
            }
            i++;
        }
        return i; // retourne le rang du 1er '=' si good
    }
    return 0;
}

struct ast_redir parse_redirection(struct lexer *lexer)
{
    if (!is_redir(lexer_peek(lexer).type))
    {
        errx(2, "Syntax error: unexpected token in redirection near token %s",
             lexer_peek(lexer).word);
    }
    struct ast_redir redir;
    struct token tok = lexer_peek(lexer);
    if (tok.type == TOKEN_IO_NUMBER)
    {
        redir.io_number = atoi(tok.word);
        lexer_pop(lexer);
        tok = lexer_peek(lexer);
    }
    else
    {
        if (is_redir_left(tok.type))
        {
            redir.io_number = 0;
        }
        else
        {
            redir.io_number = 1;
        }
    }
    redir.redir_type = tok.type;
    lexer_pop(lexer);
    tok = lexer_peek(lexer);
    if (tok.type == TOKEN_EOFI)
    {
        errx(2, "Syntax error: unexpected token in redirection near token %s",
             tok.word);
    }
    redir.filename = tok.word;
    tok = lexer_pop(lexer);
    return redir;
}

struct ast_base *parse_input(struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);
    if (tok.type == TOKEN_EOFI || tok.type == TOKEN_NEWLINE)
    {
        if (tok.type == TOKEN_NEWLINE)
        {
            tok = lexer_pop(lexer);
        }
        return NULL;
    }
    struct ast_base *tree = parse_rule(RULE_LIST, lexer);

    if (!check_token(lexer, TOKEN_EOFI) && !check_token(lexer, TOKEN_NEWLINE))
    {
        errx(2, "Syntax error: unexpected token in input near token %s",
             lexer_peek(lexer).word);
    }
    lexer_pop(lexer);
    return tree;
}

static struct ast_base *parse_list(struct lexer *lexer)
{
    struct ast_list *tree = malloc(sizeof(struct ast_list));
    tree->base.type = AST_LIST;

    size_t i = 2;
    tree->children = malloc(sizeof(struct ast_base *) * i);
    tree->children[0] = parse_rule(RULE_AND_OR, lexer);

    struct token tok = lexer_peek(lexer);
    while (tok.type == TOKEN_SEMICOL)
    {
        lexer_pop(lexer);
        tok = lexer_peek(lexer);
        if (first_command(tok))
        {
            i++;
            tree->children =
                realloc(tree->children, i * sizeof(struct ast_base *));
            tree->children[i - 2] = parse_rule(RULE_AND_OR, lexer);
        }
        else
        {
            break;
        }
        tok = lexer_peek(lexer);
    }
    tree->children[i - 1] = NULL;
    return (struct ast_base *)tree;
}

static struct ast_base *parse_and_or(struct lexer *lexer)
{
    struct ast_base *tree = parse_rule(RULE_PIPELINE, lexer);
    struct token tok = lexer_peek(lexer);
    while (tok.type == TOKEN_OR || tok.type == TOKEN_AND)
    {
        struct ast_cond *new = malloc(sizeof(struct ast_cond));
        new->left = tree;
        if (tok.type == TOKEN_OR)
        {
            new->base.type = AST_OR;
        }
        else if (tok.type == TOKEN_AND)
        {
            new->base.type = AST_AND;
        }
        else
        {
            break;
        }
        lexer_pop(lexer);
        new->right = parse_rule(RULE_PIPELINE, lexer);
        tree = (struct ast_base *)new;
        tok = lexer_peek(lexer);
    }
    return tree;
}

static struct ast_base *parse_pipeline(struct lexer *lexer)
{
    struct ast_base *tree;
    struct token tok = lexer_peek(lexer);
    int neg = 0;
    if (tok.type == TOKEN_NEG)
    {
        neg = 1;
        lexer_pop(lexer);
    }
    tree = parse_rule(RULE_COMMAND, lexer);
    tok = lexer_peek(lexer);
    while (tok.type == TOKEN_PIPE)
    {
        lexer_pop(lexer);
        skip_newlines(lexer);
        struct ast_pipeline *new = malloc(sizeof(struct ast_pipeline));
        new->base.type = AST_PIPELINE;
        new->left = tree;
        new->right = parse_rule(RULE_COMMAND, lexer);
        tree = (struct ast_base *)new;
        tok = lexer_peek(lexer);
    }
    if (neg)
    {
        struct ast_neg *new = malloc(sizeof(struct ast_neg));
        new->base.type = AST_NEG;
        new->child = tree;
        tree = (struct ast_base *)new;
    }
    return (struct ast_base *)tree;
}

static struct ast_base *parse_function(struct lexer *lexer)
{
    struct ast_func *func = malloc(sizeof(struct ast_func));
    func->base.type = AST_FUNC;
    struct token tok = lexer_pop(lexer);
    func->name = tok.word;
    tok = lexer_pop(lexer);
    if (tok.type != TOKEN_LEFT_PAR)
    {
        errx(2, "Syntax error: unexpected token in function near token %s",
             tok.word);
    }
    tok = lexer_pop(lexer);
    if (tok.type != TOKEN_RIGHT_PAR)
    {
        errx(2, "Syntax error: unexpected token in function near token %s",
             tok.word);
    }
    skip_newlines(lexer);
    func->start = parse_rule(RULE_SHELL_COMMAND, lexer);
    add_func_to_context(func->name, func->start, 0);
    return (struct ast_base *)func;
}

static struct ast_base *parse_command(struct lexer *lexer)
{
    struct ast_base *child;
    struct token tok = lexer_peek(lexer);
    if (tok.type == TOKEN_IF || tok.type == TOKEN_WHILE
        || tok.type == TOKEN_UNTIL || tok.type == TOKEN_FOR
        || tok.type == TOKEN_LEFT_BRA || tok.type == TOKEN_LEFT_PAR
        || tok.type == TOKEN_CASE)
    {
        child = parse_rule(RULE_SHELL_COMMAND, lexer);
    }
    else if (tok.type == TOKEN_WORD && lexer->next_tok.type == TOKEN_LEFT_PAR)
    {
        child = parse_rule(RULE_FUNCTION, lexer);
    }
    else if (tok.type == TOKEN_WORD || is_redir(tok.type))
    {
        child = parse_rule(RULE_SIMPLE_COMMAND, lexer);
    }
    else
    {
        errx(2, "Syntax error: unexpected token in command near token %s",
             tok.word);
    }
    if (is_redir(lexer_peek(lexer).type))
    {
        int max_size = 2;
        struct ast_redir_handle *hredir =
            calloc(1, sizeof(struct ast_redir_handle));
        hredir->base.type = AST_REDIR;
        hredir->child = child;
        hredir->redir = calloc(2, sizeof(struct ast_redir));
        while (is_redir(lexer_peek(lexer).type))
        {
            if (hredir->size + 1 >= max_size)
            {
                max_size *= 2;
                hredir->redir =
                    realloc(hredir->redir, max_size * sizeof(struct ast_redir));
            }
            struct ast_redir redir = parse_redirection(lexer);
            hredir->redir[hredir->size++] = redir;
        }
        return (struct ast_base *)hredir;
    }
    return child;
}

static struct ast_redir parse_element(struct lexer *lexer, char **buf)
{
    struct token tok = lexer_peek(lexer);
    if (is_redir(lexer_peek(lexer).type))
    {
        return parse_redirection(lexer);
    }
    else if (!is_terminal_token(lexer_peek(lexer).type))
    {
        lexer_pop(lexer);
        *buf = tok.word;
        struct ast_redir temp;
        temp.io_number = -9;
        return temp;
    }
    else
    {
        errx(2, "Syntax error: unexpected token in element near token %s",
             tok.word);
    }
}

static struct ast_redir_handle *
simple_command_redir(struct ast_redir_handle *hredir, struct lexer *lexer,
                     int *max_redir_size)
{
    struct token tok = lexer_peek(lexer);
    hredir->redir = calloc(2, sizeof(struct ast_redir));
    hredir->assignment_dict = calloc(2, sizeof(struct ast_redir));
    int rank = 0; // 'foo=bar' => rank of equal is 3
    int max_dict_size = 2;
    while (is_redir(tok.type) || (rank = is_assignment(tok)))
    {
        if (is_redir(tok.type))
        {
            if (hredir->size + 1 >= *max_redir_size)
            {
                (*max_redir_size) *= 2;
                hredir->redir = realloc(
                    hredir->redir, *max_redir_size * sizeof(struct ast_redir));
            }
            struct ast_redir redir = parse_redirection(lexer);
            hredir->redir[hredir->size++] = redir;
            tok = lexer_peek(lexer);
        }
        else
        {
            lexer_pop(lexer);

            if (hredir->dict_size + 1 >= max_dict_size)
            {
                (max_dict_size) *= 2;
                hredir->assignment_dict =
                    realloc(hredir->assignment_dict,
                            max_dict_size * sizeof(struct ast_redir));
            }

            tok.word[rank] = '\0';
            char *ptr_value = tok.word + rank + 1;
            char *value_string = calloc(strlen(ptr_value) + 1, sizeof(char));
            value_string = strcpy(value_string, ptr_value);

            add_string_to_context(value_string);

            struct dict new_pair;
            new_pair.key = tok.word;
            new_pair.value = value_string;
            hredir->assignment_dict[hredir->dict_size++] = new_pair;
            tok = lexer_peek(lexer);
        }
    }
    return hredir;
}

static struct ast_base *
simple_command_return(struct ast_simple_command *command,
                      struct ast_redir_handle *hredir, int *size)
{
    command->args[*size] = NULL;
    if (hredir->size != 0 || hredir->dict_size != 0)
    {
        if (*size == 0)
        {
            free(command->args);
            free(command);
        }
        else
        {
            hredir->child = (struct ast_base *)command;
        }
        return (struct ast_base *)hredir;
    }
    free(hredir);
    return (struct ast_base *)command;
}

static struct ast_base *parse_simple_command(struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);
    struct ast_simple_command *command =
        calloc(1, sizeof(struct ast_simple_command));
    command->base.type = AST_CMD;
    struct ast_redir_handle *h = calloc(1, sizeof(struct ast_redir_handle));
    h->base.type = AST_REDIR;
    int max_redir_size = 2;
    int hasprefix = 0;
    if (is_redir(tok.type) || is_assignment(tok))
    {
        h = simple_command_redir(h, lexer, &max_redir_size);
        tok = lexer_peek(lexer);
        hasprefix = 1;
    }
    int size = 0;
    int max_size = 2;
    command->args = malloc(sizeof(char *) * max_size);
    if (tok.type == TOKEN_WORD)
    {
        command->args[size++] = lexer_peek(lexer).word;
        lexer_pop(lexer);
    }
    else if (!hasprefix)
    {
        errx(2, "Syntax error: unexpected token %s", tok.word);
    }
    while (!is_terminal_token(lexer_peek(lexer).type)
           || is_redir(lexer_peek(lexer).type))
    {
        char *buf = NULL;
        struct ast_redir redir = parse_element(lexer, &buf);
        if (redir.io_number == -9)
        {
            if (size + 1 >= max_size)
            {
                max_size *= 2;
                command->args =
                    realloc(command->args, max_size * sizeof(char *));
            }
            command->args[size++] = buf;
        }
        else
        {
            if (h->size == 0)
            {
                h->redir = calloc(2, sizeof(struct ast_redir));
            }
            if (h->size + 1 >= max_redir_size)
            {
                max_redir_size *= 2;
                h->redir = realloc(h->redir,
                                   max_redir_size * sizeof(struct ast_redir));
            }
            h->redir[h->size++] = redir;
        }
    }
    return simple_command_return(command, h, &size);
}

struct ast_base *parse_subshell(struct lexer *lexer)
{
    struct token tok = lexer_pop(lexer);
    if (tok.type != TOKEN_LEFT_PAR)
    {
        errx(2, "Syntax error: unexpected token in subshell near token %s",
             tok.word);
    }
    struct ast_subshell *shell = malloc(sizeof(struct ast_subshell));
    shell->base.type = AST_SUBSHELL;
    shell->content = parse_rule(RULE_COMPLIST, lexer);
    tok = lexer_pop(lexer);
    if (tok.type != TOKEN_RIGHT_PAR)
    {
        errx(2, "Syntax error: unexpected token in subshell near token %s",
             tok.word);
    }
    return (struct ast_base *)shell;
}

struct ast_base *parse_shell_command(struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);
    struct ast_base *child;
    if (tok.type == TOKEN_IF)
    {
        child = parse_rule(RULE_IF, lexer);
    }
    else if (tok.type == TOKEN_WHILE)
    {
        child = parse_rule(RULE_WHILE, lexer);
    }
    else if (tok.type == TOKEN_UNTIL)
    {
        child = parse_rule(RULE_UNTIL, lexer);
    }
    else if (tok.type == TOKEN_FOR)
    {
        child = parse_rule(RULE_FOR, lexer);
    }
    else if (tok.type == TOKEN_LEFT_BRA)
    {
        tok = lexer_pop(lexer);
        child = parse_rule(RULE_COMPLIST, lexer);
        tok = lexer_pop(lexer);
        if (tok.type != TOKEN_RIGHT_BRA)
        {
            errx(2, "Syntax error: unexpected token in shell cmd near token %s",
                 tok.word);
        }
    }
    else if (tok.type == TOKEN_LEFT_PAR)
    {
        child = parse_rule(RULE_SUBSHELL, lexer);
    }
    else if (tok.type == TOKEN_CASE)
    {
        child = parse_rule(RULE_CASE, lexer);
    }
    else
    {
        errx(2, "Syntax error: unexpected token in shell cmd near token %s",
             tok.word);
    }
    return (struct ast_base *)child;
}

static struct ast_base *parse_rule_if(struct lexer *lexer)
{
    struct token tok = lexer_pop(lexer);
    if (tok.type != TOKEN_IF)
    {
        errx(2, "Syntax error: unexpected token in rule if next token %s",
             tok.word);
    }

    struct ast_if *tree = calloc(1, sizeof(struct ast_if));
    tree->base.type = AST_IF;
    tree->condition = parse_rule(RULE_COMPLIST, lexer);
    tok = lexer_pop(lexer);
    if (tok.type != TOKEN_THEN)
    {
        errx(2, "Syntax error: unexpected token in rule if next token %s",
             tok.word);
    }

    tree->then_body = parse_rule(RULE_COMPLIST, lexer);

    tok = lexer_peek(lexer);
    if (tok.type == TOKEN_ELSE || tok.type == TOKEN_ELIF)
    {
        tree->else_body = parse_rule(RULE_ELSE, lexer);
    }

    tok = lexer_pop(lexer);
    if (tok.type != TOKEN_FI)
    {
        errx(2, "Syntax error: unexpected token in rule if next token %s",
             tok.word);
    }
    return (struct ast_base *)tree;
}

static struct ast_base *parse_else(struct lexer *lexer)
{
    struct token tok = lexer_pop(lexer);
    if (tok.type == TOKEN_ELSE)
    {
        return parse_rule(RULE_COMPLIST, lexer);
    }
    else
    {
        struct ast_if *tree = malloc(sizeof(struct ast_if));
        tree->base.type = AST_IF;
        tree->condition = parse_rule(RULE_COMPLIST, lexer);

        tok = lexer_pop(lexer);
        if (tok.type != TOKEN_THEN)
        {
            errx(2, "Syntax error: unexpected token in rule else next token %s",
                 tok.word);
        }

        tree->then_body = parse_rule(RULE_COMPLIST, lexer);

        tok = lexer_peek(lexer);
        if (tok.type == TOKEN_ELSE || tok.type == TOKEN_ELIF)
        {
            tree->else_body = parse_rule(RULE_ELSE, lexer);
        }
        return (struct ast_base *)tree;
    }
}

static struct ast_base *parse_rule_while(struct lexer *lexer)
{
    struct token tok = lexer_pop(lexer);
    if (tok.type != TOKEN_WHILE)
    {
        errx(2, "Syntax error: unexpected token in rule while next token %s",
             tok.word);
    }

    struct ast_loop *tree = calloc(1, sizeof(struct ast_loop));
    tree->base.type = AST_WHILE;
    tree->condition = parse_rule(RULE_COMPLIST, lexer);

    tok = lexer_pop(lexer);
    if (tok.type != TOKEN_DO)
    {
        errx(2, "Syntax error: unexpected token in rule while next token %s",
             tok.word);
    }

    tree->body = parse_rule(RULE_COMPLIST, lexer);

    tok = lexer_pop(lexer);
    if (tok.type != TOKEN_DONE)
    {
        errx(2, "Syntax error: unexpected token in rule while next token %s",
             tok.word);
    }
    return (struct ast_base *)tree;
}

static struct ast_base *parse_rule_until(struct lexer *lexer)
{
    struct token tok = lexer_pop(lexer);
    if (tok.type != TOKEN_UNTIL)
    {
        errx(2, "Syntax error: unexpected token in rule until next token %s",
             tok.word);
    }

    struct ast_loop *tree = calloc(1, sizeof(struct ast_loop));
    tree->base.type = AST_UNTIL;
    tree->condition = parse_rule(RULE_COMPLIST, lexer);

    tok = lexer_pop(lexer);
    if (tok.type != TOKEN_DO)
    {
        errx(2, "Syntax error: unexpected token in rule until next token %s",
             tok.word);
    }

    tree->body = parse_rule(RULE_COMPLIST, lexer);

    tok = lexer_pop(lexer);
    if (tok.type != TOKEN_DONE)
    {
        errx(2, "Syntax error: unexpected token in rule until next token %s",
             tok.word);
    }
    return (struct ast_base *)tree;
}

static struct ast_loop *create_for(struct lexer *lexer)
{
    struct token tok = lexer_pop(lexer);
    if (tok.type != TOKEN_FOR)
    {
        errx(2, "Syntax error: unexpected token in rule for next token");
    }
    struct ast_loop *tree = calloc(1, sizeof(struct ast_loop));
    tree->base.type = AST_FOR;
    return tree;
}

static struct ast_for_list *create_forlist(struct lexer *lexer)
{
    struct token tok = lexer_pop(lexer);
    struct ast_for_list *cond = calloc(1, sizeof(struct ast_for_list));
    cond->base.type = AST_FORLIST;
    cond->word = tok.word;
    cond->list = calloc(1, sizeof(char *));
    return cond;
}

static struct ast_for_list *realloc_forlist(int size, int *max_size,
                                            struct ast_for_list *cond,
                                            struct lexer *lexer)
{
    if (size + 1 >= *max_size)
    {
        *max_size *= 2;
        cond->list = realloc(cond->list, *max_size * sizeof(char *));
    }
    lexer_pop(lexer);
    return cond;
}

static struct ast_base *parse_rule_for(struct lexer *lexer)
{
    struct ast_loop *tree = create_for(lexer);
    struct ast_for_list *cond = create_forlist(lexer);
    struct token tok = lexer_peek(lexer);
    if (tok.type == TOKEN_SEMICOL)
    {
        lexer_pop(lexer);
        tok = lexer_peek(lexer);
    }
    else
    {
        skip_newlines(lexer);
        tok = lexer_peek(lexer);
        if (tok.type == TOKEN_IN)
        {
            lexer_pop(lexer);
            tok = lexer_peek(lexer);
            int size = 0;
            int max_size = 2;
            while (!is_terminal_token(tok.type))
            {
                cond = realloc_forlist(size, &max_size, cond, lexer);
                cond->list[size++] = tok.word;
                tok = lexer_peek(lexer);
            }
            if (size > 0)
            {
                cond->list[size] = NULL;
            }
            if (tok.type != TOKEN_NEWLINE && tok.type != TOKEN_SEMICOL)
            {
                errx(2, "Syntax error: unexpected next token in rule for");
            }
            lexer_pop(lexer);
        }
    }
    skip_newlines(lexer);
    tree->condition = (struct ast_base *)cond;
    tok = lexer_peek(lexer);
    if (tok.type != TOKEN_DO)
    {
        errx(2, "Syntax error: unexpected token in rule for next token ");
    }
    lexer_pop(lexer);
    tree->body = parse_rule(RULE_COMPLIST, lexer);
    tok = lexer_pop(lexer);
    if (tok.type != TOKEN_DONE)
    {
        errx(2, "Syntax error: unexpected token in rule for next token");
    }
    lexer_pop(lexer);
    return (struct ast_base *)tree;
}

static struct ast_base *parse_complist(struct lexer *lexer)
{
    struct ast_list *tree = malloc(sizeof(struct ast_list));
    tree->base.type = AST_LIST;

    size_t i = 2;
    tree->children = malloc(sizeof(struct ast_base *) * i);

    skip_newlines(lexer);
    tree->children[0] = parse_rule(RULE_AND_OR, lexer);
    struct token tok = lexer_peek(lexer);

    while (tok.type == TOKEN_SEMICOL || tok.type == TOKEN_NEWLINE)
    {
        lexer_pop(lexer);
        skip_newlines(lexer);
        tok = lexer_peek(lexer);

        if (first_command(tok))
        {
            i++;
            tree->children =
                realloc(tree->children, i * sizeof(struct ast_base *));
            tree->children[i - 2] = parse_rule(RULE_AND_OR, lexer);
        }
        else
        {
            break;
        }
        tok = lexer_peek(lexer);
    }
    skip_newlines(lexer);
    tree->children[i - 1] = NULL;
    return (struct ast_base *)tree;
}

// char **values, ast_base *body
struct case_handler parse_case_item(struct lexer *lexer)
{
    struct case_handler res;
    struct token tok = lexer_pop(lexer);
    if (tok.type == TOKEN_LEFT_PAR)
    {
        tok = lexer_pop(lexer);
    }
    int size = 0;
    int max_size = 2;
    res.values = calloc(max_size, sizeof(char *));
    res.values[size++] = tok.word;
    tok = lexer_pop(lexer);
    while (tok.type == TOKEN_PIPE)
    {
        if (size + 1 >= max_size)
        {
            max_size *= 2;
            res.values = realloc(res.values, max_size * sizeof(char *));
        }
        tok = lexer_pop(lexer);
        res.values[size++] = tok.word;
        tok = lexer_pop(lexer);
    }
    res.values[size] = NULL;
    if (tok.type != TOKEN_RIGHT_PAR)
        errx(2, "unexpected case token");
    skip_newlines(lexer);
    tok = lexer_peek(lexer);
    if (first_command(tok))
        res.body = parse_rule(RULE_COMPLIST, lexer);
    else
        res.body = NULL;
    return res;
}

// base word cases nb cases
void parse_case_clause(struct lexer *lexer, struct ast_case *tree)
{
    size_t max_size = 2;
    tree->cases = calloc(2, sizeof(struct case_handler));
    tree->nb_cases = 0;
    struct token tok = lexer_peek(lexer);
    while (tok.type != TOKEN_ESAC && tok.type != TOKEN_EOFI)
    {
        if (tree->nb_cases + 1 >= max_size)
        {
            max_size *= 2;
            tree->cases =
                realloc(tree->cases, max_size * sizeof(struct case_handler));
        }
        tree->cases[tree->nb_cases++] = parse_case_item(lexer);
        tok = lexer_peek(lexer);
        if (tok.type == TOKEN_DBL_SEMICOL)
        {
            lexer_pop(lexer);
            tok = lexer_peek(lexer);
        }
        skip_newlines(lexer);
        tok = lexer_peek(lexer);
    }
}

struct ast_base *parse_rule_case(struct lexer *lexer)
{
    struct token tok = lexer_pop(lexer);
    if (tok.type != TOKEN_CASE)
    {
        errx(2, "unexpected token in rule case");
    }
    struct ast_case *tree = malloc(sizeof(struct ast_case));
    tree->base.type = AST_CASE;
    tok = lexer_pop(lexer);
    tree->word = tok.word;
    skip_newlines(lexer);
    tok = lexer_pop(lexer);
    if (tok.type != TOKEN_IN)
    {
        errx(2, "unexpected token in rule case");
    }
    skip_newlines(lexer);
    tok = lexer_peek(lexer);
    if (tok.type != TOKEN_ESAC)
    {
        parse_case_clause(lexer, tree);
    }
    tok = lexer_peek(lexer);
    if (tok.type != TOKEN_ESAC)
    {
        errx(2, "unexpected token in rule case");
    }
    lexer_pop(lexer);
    return (struct ast_base *)tree;
}

typedef struct ast_base *(*rule_list)(struct lexer *lexer);

static const rule_list fnc[] = {
    [RULE_INPUT] = parse_input,
    [RULE_LIST] = parse_list,
    [RULE_AND_OR] = parse_and_or,
    [RULE_PIPELINE] = parse_pipeline,
    [RULE_COMMAND] = parse_command,
    [RULE_SIMPLE_COMMAND] = parse_simple_command,
    [RULE_SHELL_COMMAND] = parse_shell_command,
    [RULE_IF] = parse_rule_if,
    [RULE_ELSE] = parse_else,
    [RULE_COMPLIST] = parse_complist,
    [RULE_WHILE] = parse_rule_while,
    [RULE_UNTIL] = parse_rule_until,
    [RULE_FOR] = parse_rule_for,
    [RULE_FUNCTION] = parse_function,
    [RULE_SUBSHELL] = parse_subshell,
    [RULE_CASE] = parse_rule_case,
};

struct ast_base *parse_rule(enum parser_rule r, struct lexer *lexer)
{
    return fnc[r](lexer);
}
