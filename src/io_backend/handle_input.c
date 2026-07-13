#define _POSIX_C_SOURCE 200809L
#include "handle_input.h"

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils/context.h"
#include "utils/myutils.h"

FILE *getinput(int argc, char **argv)
{
    FILE *input = NULL;
    struct context *ctx = get_context();
    ctx->buffer = NULL;
    if (argc >= 3 && !strcmp(argv[1], "-c"))
    {
        size_t length = strlen(argv[2]) + 1;
        char *str_buffer = malloc(length + 1);
        if (!str_buffer)
        {
            perror("Failed to allocate memory for input buffer");
            exit(1);
        }

        strncpy(str_buffer, argv[2], length);
        str_buffer[length] = '\0';

        ctx->arg_start = 3;
        ctx->buffer = str_buffer;

        input = fmemopen(str_buffer, length + 1, "r");

        if (!input)
        {
            perror("Failed to create memory stream");
            exit(127);
        }
    }
    else if (argc > 1)
    {
        ctx->arg_start = 2;
        input = fopen(argv[1], "r");
        if (!input)
        {
            perror("Failed to open file");
            exit(127);
        }
    }
    else
    {
        ctx->arg_start = -1;
        setvbuf(stdin, NULL, _IONBF, 0);
        input = stdin;
        if (!input)
        {
            perror("Failed to open stdin");
            exit(127);
        }
    }

    return input;
}

// -- LEXER FUNCTIONS --

static void skip_commented(FILE *fd, char *current)
{
    *current = fgetc(fd);
    ungetc(*current, fd);
    if (*current == '#')
    {
        while (*current != '\n' && *current != EOF)
        {
            *current = fgetc(fd);
        }
    }
}

static void skip_escaped(char *res, char *current, FILE *fd, size_t *size)
{
    struct context *ctx = get_context();
    ctx->escaped = 1;

    char check_newline = fgetc(fd);
    check_newline = fgetc(fd);

    ungetc(check_newline, fd);
    ungetc(check_newline, fd);

    if (check_newline == '\n')
    {
        *current = fgetc(fd);
        *current = fgetc(fd);
    }
    else
    {
        res[*size] = *current;
        *size += 1;
        *current = fgetc(fd);
        *current = fgetc(fd);
        res[*size] = *current;
        *size += 1;
    }
}

static void escape_mid_separator(char *current, FILE *fd)
{
    char check_newline = fgetc(fd);
    ungetc(check_newline, fd);

    if (check_newline == '\n')
    {
        *current = fgetc(fd);
        *current = fgetc(fd);
    }
}

char *handle_par(size_t *max_size, char *res, size_t *size)
{
    struct context *ctx = get_context();
    int count = 1;
    FILE *fd = ctx->fd;
    fgetc(fd);
    char current = fgetc(fd);
    do
    {
        if (*size + 1 >= *max_size)
        {
            *max_size *= 2;
            res = realloc(res, *max_size * sizeof(char));
        }
        if (current == EOF)
        {
            errx(2, "io_backend : unclosed quote !\n");
        }
        if (current == '\\')
        {
            res[(*size)++] = current;
            current = fgetc(fd);
        }
        if (current == '(')
        {
            count++;
        }
        if (current == ')')
        {
            count--;
        }
        res[(*size)++] = current;
        current = fgetc(fd);
    } while (count != 0);
    return res;
}

char *handle_quotes(size_t *max_size, char *res, size_t *size, char delim)
{
    struct context *ctx = get_context();
    ctx->escaped = 1;
    FILE *fd = ctx->fd;
    fgetc(fd);
    char current = fgetc(fd);
    do
    {
        if (*size + 1 >= *max_size)
        {
            *max_size *= 2;
            res = realloc(res, *max_size * sizeof(char));
        }
        if (current == EOF)
        {
            errx(2, "io_backend : unclosed quote !\n");
        }
        if (current == '\\')
        {
            res[(*size)++] = current;
            current = fgetc(fd);
        }
        res[(*size)++] = current;
        current = fgetc(fd);
    } while (current != delim);
    res[(*size)++] = current;
    return res;
}

static char *handle_separator(char *current, char *res, FILE *fd)
{
    if (is_terminal(*current))
    {
        fgetc(fd);
        if (*current == ';')
        {
            res[0] = ';';
            *current = fgetc(fd);
            if (*current == ';')
            {
                res[1] = ';';
                res[2] = 0;
            }
            else
            {
                ungetc(*current, fd);
                res[1] = 0;
            }
        }
        else if (*current != EOF)
        {
            res[0] = *current;
            res[1] = 0;
        }
        else
        {
            res[0] = 0;
        }
    }
    else
    {
        *current = fgetc(fd);
        res[0] = *current;
        *current = fgetc(fd);
        if (*current == '\\')
        {
            escape_mid_separator(current, fd);
        }
        if (is_terminal_component(*current) && !is_terminal(*current))
        {
            res[1] = *current;
            res[2] = 0;
        }
        else
        {
            ungetc(*current, fd);
        }
    }
    add_string_to_context(res);
    return res;
}

char *skip_spaces(FILE *fd, int *space_flag, char *res)
{
    fgetc(fd);
    free(res);
    *space_flag = 1; // space_flag matters for io numbers
    return get_next_word(fd, space_flag);
}

char *get_next_word(FILE *fd, int *space_flag)
{
    struct context *ctx = get_context();
    ctx->escaped = 0;
    ctx->quoted = 0;
    ctx->fd = fd;
    char current = 1;
    size_t size = 0;
    size_t max_size = 4;
    char *res = calloc(4, sizeof(char));
    skip_commented(fd, &current); // if # -> skip until newline
    while (current != EOF)
    {
        if (size + 1 >= max_size)
        {
            max_size *= 2;
            res = realloc(res, max_size * sizeof(char));
        }
        current = fgetc(fd);
        ungetc(current, fd);
        if (current == '\\')
        {
            skip_escaped(res, &current, fd, &size);
        }
        else if (current == '$')
        {
            current = fgetc(fd);
            res[size++] = current;
        }
        else if (isblank(current) && size == 0)
        {
            return skip_spaces(fd, space_flag, res);
        }
        else if (is_terminal_component(current))
        {
            if (size == 0)
            {
                return handle_separator(&current, res, fd);
            }
            else if (res[size - 1] == '$' && current == '(')
            {
                res[size++] = current;
                res = handle_par(&max_size, res, &size);
            }
            else
                break;
        }
        else if (current == '\'' || current == '\"' || current == '`')
        {
            ungetc(current, fd);
            res = handle_quotes(&max_size, res, &size, current);
        }
        else
        {
            res[size++] = current;
            current = fgetc(fd);
        }
    }
    res[size] = 0;
    add_string_to_context(res);
    return res;
}
