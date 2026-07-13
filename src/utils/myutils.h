#ifndef MYUTILS_H
#define MYUTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "lexer/lexer.h"

struct cview
{
    const char *data;
    size_t size;
};

bool is_terminal(char c);

bool is_terminal_component(char c);

bool is_special_character(char c);

int is_terminal_token(enum token_type type);

int is_redir(enum token_type type);

int is_redir_left(enum token_type type);

#endif /* !MYUTILS_H */
