#include "lexer.h"

#include <ctype.h>
#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io_backend/handle_input.h"
#include "utils/context.h"
#include "utils/myutils.h"

static char *words[] = { "if",    "then",  "else", "elif", "fi",   ";",    "\n",
                         "&&",    "||",    "for",  "in",   "\0",   "|",    "!",
                         "while", "until", "do",   "done", "case", "esac", ";;",
                         "(",     ")",     "{",    "}",    "<",    "<&",   ">",
                         ">&",    ">|",    ">>",   "<>",   NULL };

struct lexer *lexer_new(FILE *fd)
{
    struct lexer *res = malloc(sizeof(struct lexer));
    res->fd = fd;
    res->tok.type = TOKEN_FIRST;
    res->next_tok.type = TOKEN_FIRST;
    return res;
}

void lexer_free(struct lexer *lexer)
{
    if (lexer)
        free(lexer);
}

struct token lexer_next_token(struct lexer *lexer)
{
    struct context *ctx = get_context();
    struct token t;
    int space_flag = 0;
    char *text = get_next_word(lexer->fd, &space_flag);
    t.type = TOKEN_WORD;
    if (lexer->next_tok.type != TOKEN_WORD)
    {
        for (size_t i = 0; i < ctx->nb_aliases; i++)
        {
            if (strcmp(text, ctx->aliases[i].name) == 0)
            {
                char *temp = ctx->aliases[i].value;
                int len = strlen(temp) - 1;
                while (len >= 0)
                {
                    ungetc(temp[len--], ctx->fd);
                }
                return lexer_next_token(lexer);
            }
        }
    }
    for (int i = 0; words[i]; i++)
    {
        if (!strcmp(text, words[i]))
        {
            t.type = i;
            break;
        }
    }
    t.word = text;
    t.len = strlen(text);
    t.skipped_space = space_flag;
    lexer->tok = lexer->next_tok;
    lexer->next_tok = t;
    return t;
}

void transform_token(struct lexer *lexer)
{
    struct token tok = lexer->tok;
    if (tok.type == TOKEN_WORD && strlen(tok.word) == 1 && isdigit(tok.word[0])
        && is_redir(lexer->next_tok.type) && lexer->next_tok.skipped_space == 0)
        lexer->tok.type = TOKEN_IO_NUMBER;
}

struct token lexer_peek(struct lexer *lexer)
{
    if (lexer->tok.type == TOKEN_FIRST)
    {
        lexer_next_token(lexer);
        lexer_next_token(lexer);
    }
    transform_token(lexer);
    return lexer->tok;
}

struct token lexer_pop(struct lexer *lexer)
{
    if (lexer->tok.type == TOKEN_FIRST)
    {
        lexer_next_token(lexer);
        lexer_next_token(lexer);
    }
    struct token t = lexer->tok;
    transform_token(lexer);
    lexer_next_token(lexer);
    transform_token(lexer);
    return t;
}

/*void lexer_aux(struct token *t, char *text)
{
    if (!strcmp(text, "for"))
    {
        t->type = TOKEN_FOR;
    }
    else if (!strcmp(text, "in"))
    {
        t->type = TOKEN_IN;
    }
    else if (!strcmp(text, "done"))
    {
        t->type = TOKEN_DONE;
    }
    else if (!strcmp(text, "until"))
    {
        t->type = TOKEN_UNTIL;
    }
    else if (!strcmp(text, "&&"))
    {
        t->type = TOKEN_AND;
    }
    else if (!strcmp(text, "||"))
    {
        t->type = TOKEN_OR;
    }
    else if (!strcmp(text, "|"))
    {
        t->type = TOKEN_PIPE;
    }
    else if (!strcmp(text, "!"))
    {
        t->type = TOKEN_NEG;
    }
    else if (!strcmp(text, "<"))
    {
        t->type = REDIR_LEFT;
    }
    else if (!strcmp(text, "<&"))
    {
        t->type = REDIR_LEFT_STD;
    }
    else if (!strcmp(text, ">"))
    {
        t->type = REDIR_RIGHT;
    }
    else if (!strcmp(text, ">&"))
    {
        t->type = REDIR_RIGHT_STD;
    }
    else if (!strcmp(text, ">|"))
    {
        t->type = REDIR_RIGHT_FORCE;
    }
    else if (!strcmp(text, "<>"))
    {
        t->type = REDIR_OPEN_FD;
    }
    else if (!strcmp(text, ">>"))
    {
        t->type = REDIR_RIGHT_APPEND;
    }
    else if (is_terminal_component(text[0]) && !is_terminal(text[0]))
    {
        errx(2, "Unexpected token");
    }
    else if (!strcmp(text, "\0"))
    {
        t->type = TOKEN_EOFI;
    }
    else
    {
        t->type = TOKEN_WORD;
    }
}
*/

/*else if (!strcmp(text, "if"))
{
    t.type = TOKEN_IF;
}
else if (!strcmp(text, "fi"))
{
    t.type = TOKEN_FI;
}
else if (!strcmp(text, "else"))
{
    t.type = TOKEN_ELSE;
}
else if (!strcmp(text, "elif"))
{
    t.type = TOKEN_ELIF;
}
else if (!strcmp(text, "then"))
{
    t.type = TOKEN_THEN;
}
else if (!strcmp(text, ";"))
{
    t.type = TOKEN_SEMICOL;
}
else if (!strcmp(text, "\n"))
{
    t.type = TOKEN_NEWLINE;
}
else if (!strcmp(text, "while"))
{
    t.type = TOKEN_WHILE;
}
else if (!strcmp(text, "do"))
{
    t.type = TOKEN_DO;
}
else
{
    lexer_aux(&t, text);
    }*/
