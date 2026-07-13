#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>
#include <stdio.h>

enum token_type
{
    TOKEN_IF,
    TOKEN_THEN,
    TOKEN_ELSE,
    TOKEN_ELIF,
    TOKEN_FI,

    TOKEN_SEMICOL, //   ;
    TOKEN_NEWLINE, //   \n

    TOKEN_AND,
    TOKEN_OR,

    TOKEN_FOR,
    TOKEN_IN,

    TOKEN_EOFI, //      EOF
    TOKEN_PIPE, //      |
    TOKEN_NEG,

    TOKEN_WHILE,
    TOKEN_UNTIL,
    TOKEN_DO,
    TOKEN_DONE,
    TOKEN_CASE,
    TOKEN_ESAC,
    TOKEN_DBL_SEMICOL,

    TOKEN_LEFT_PAR,
    TOKEN_RIGHT_PAR,

    TOKEN_LEFT_BRA,
    TOKEN_RIGHT_BRA,

    REDIR_LEFT, //      <
    REDIR_LEFT_STD, //  <&
    REDIR_RIGHT, //     >
    REDIR_RIGHT_STD, // &>
    REDIR_RIGHT_FORCE, // >|
    REDIR_RIGHT_APPEND, // >>
    REDIR_OPEN_FD, //   <>
    TOKEN_IO_NUMBER,

    TOKEN_WORD,

    TOKEN_FIRST,
};

struct token
{
    enum token_type type;
    char *word; // if token type is WORD
    int len;
    int skipped_space;
};

struct lexer
{
    FILE *fd;
    struct token tok;
    struct token next_tok;
};

struct lexer *lexer_new(FILE *fd);

void lexer_free(struct lexer *lexer);

struct token lexer_next_token(struct lexer *lexer);

struct token lexer_peek(struct lexer *lexer);

struct token lexer_pop(struct lexer *lexer);

void pretty_print_token(struct lexer *lexer);

#endif /* !LEXER_H */
