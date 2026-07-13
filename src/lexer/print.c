#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

char *aux_ppt(char *word, size_t len)
{
    char *res = calloc(len, sizeof(char));
    for (size_t i = 0; i < len; i++)
    {
        res[i] = word[i];
    }
    return res;
}

void pretty_print_token_aux2(struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);
    switch (tok.type)
    {
    case TOKEN_LEFT_PAR:
        printf("type: TOKEN_LEFT_PAR\n");
        break;
    case TOKEN_RIGHT_PAR:
        printf("type: TOKEN_RIGHT_PAR\n");
        break;
    case TOKEN_LEFT_BRA:
        printf("type: TOKEN_LEFT_BRA\n");
        break;
    case TOKEN_RIGHT_BRA:
        printf("type: TOKEN_LEFT_BRA\n");
        break;
    case REDIR_LEFT:
        printf("type: REDIR_LEFT\n");
        break;
    case REDIR_LEFT_STD:
        printf("type: REDIR_LEFT_STD\n");
        break;
    case REDIR_RIGHT:
        printf("type: REDIR_RIGHT\n");
        break;
    case REDIR_RIGHT_STD:
        printf("type: REDIR_RIGHT_STD\n");
        break;
    case REDIR_RIGHT_FORCE:
        printf("type: REDIR_RIGHT_FORCE\n");
        break;
    case REDIR_OPEN_FD:
        printf("type: REDIR_OPEN_FD\n");
        break;
    default:
        fprintf(stderr, "pretty_print_token: unknown token\n");
        return;
    }
}

void pretty_print_token_aux(struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);
    switch (tok.type)
    {
    case TOKEN_FOR:
        printf("type: FOR\n");
        break;
    case TOKEN_IN:
        printf("type: IN\n");
        break;
    case TOKEN_EOFI:
        printf("type: EOF\n");
        break;
    case TOKEN_PIPE:
        printf("type: PIPE\n");
        break;
    case TOKEN_NEG:
        printf("type: NEGATI\n");
        break;
    case TOKEN_WHILE:
        printf("type: WHILE\n");
        break;
    case TOKEN_UNTIL:
        printf("type: UNTIL\n");
        break;
    case TOKEN_DO:
        printf("type: DO\n");
        break;
    case TOKEN_DONE:
        printf("type: DONE\n");
        break;
    default:
        pretty_print_token_aux2(lexer);
        return;
    }
}

void pretty_print_token(struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);
    printf("Tokens: ");
    /*while (tok.type != EOFI)
    {
        lexer_pop(lexer);*/
    switch (tok.type)
    {
    case TOKEN_IF:
        printf("type: IF\n");
        break;
    case TOKEN_THEN:
        printf("type: THEN\n");
        break;
    case TOKEN_ELSE:
        printf("type: ELSE\n");
        break;
    case TOKEN_ELIF:
        printf("type: ELIF\n");
        break;
    case TOKEN_FI:
        printf("type: FI\n");
        break;
    case TOKEN_SEMICOL:
        printf("type: SEMICOL\n");
        break;
    case TOKEN_NEWLINE:
        printf("type: NEWLINE\n");
        break;
    case TOKEN_WORD:
        printf("type: WORD, word: %s, len: %zu\n", tok.word, strlen(tok.word));
        break;
    case TOKEN_AND:
        printf("type: AND\n");
        break;
    case TOKEN_OR:
        printf("type: OR\n");
        break;
    default:
        pretty_print_token_aux(lexer);
        return;
    }
    /*  tok = lexer_peek(lexer);
  }*/
}
