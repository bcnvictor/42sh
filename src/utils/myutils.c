#include "myutils.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer/lexer.h"

bool is_terminal(char c)
{
    return (c == '\0' || c == ' ' || c == '\t' || c == '\n' || c == ';'
            || c == '(' || c == ')' || c == EOF);
}

bool is_terminal_component(char c)
{
    return (c == '\0' || c == ' ' || c == '\t' || c == '\n' || c == ';'
            || c == EOF || c == '(' || c == ')' || c == '|' || c == '&'
            || c == '<' || c == '>');
}

// fonction uniquement relevent quand on est en train de parser qqch entre des
// doublequotes !
bool is_special_character(char c)
{
    return (c == '$' || c == '\\' || c == '`');
}

// terminal token/symbols delimiting the end of a command's argument
int is_terminal_token(enum token_type type)
{
    return (is_redir(type) || type == TOKEN_SEMICOL || type == TOKEN_NEWLINE
            || type == TOKEN_EOFI || type == TOKEN_PIPE || type == TOKEN_NEG
            || type == TOKEN_AND || type == TOKEN_OR || type == TOKEN_LEFT_PAR
            || type == TOKEN_RIGHT_PAR || type == TOKEN_DBL_SEMICOL);
}

int is_redir(enum token_type type)
{
    return (type == REDIR_LEFT || type == REDIR_RIGHT || type == REDIR_LEFT_STD
            || type == REDIR_RIGHT_STD || type == REDIR_RIGHT_FORCE
            || type == REDIR_OPEN_FD || type == REDIR_RIGHT_APPEND
            || type == TOKEN_IO_NUMBER);
}

int is_redir_left(enum token_type type)
{
    return (type == REDIR_LEFT || type == REDIR_LEFT_STD
            || type == REDIR_OPEN_FD);
}

/*
struct cview string_to_cview(char *ptr_string)
{
    struct cview res_cview;
    res_cview.data = ptr_string;
    size_t counter = 0;
    if (ptr_string)
    {
        char *tmp_ptr = ptr_string;
        char current = tmp_ptr[counter];
        while (!is_terminal(current))
        {
            counter++;
            current = tmp_ptr[counter];
        }
        res_cview.size = counter;
        return res_cview;
    }
    else
    {
        fprintf(stdout, "cview: calling function on null str pointer! \n");
        res_cview.data = NULL;
        res_cview.size = 0;
        return res_cview;
    }
}

char *cview_to_string(struct cview view)
{
    char *res = calloc(view.size + 1, sizeof(char));
    size_t i = 0;
    for (; i < view.size; i++)
    {
        res[i] = view.data[i];
    }
    res[i] = '\0';
    return res;
}

int cview_cmp(struct cview view, char *str)
{
    if (strlen(str) != view.size)
        return 1;
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (view.data[i] != str[i])
            return 1;
    }
    return 0;
}

void print_cview(struct cview view)
{
    for (size_t i = 0; i < view.size; i++)
    {
        putchar(view.data[i]);
    }
}
*/
