#include "expansion.h"

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast/ast.h"
#include "utils/context.h"
#include "utils/myutils.h"

char **scan_expand_variable(char **argv)
{
    struct expansion_handle *h = malloc(sizeof(struct expansion_handle));
    memset(h, 0, sizeof(struct expansion_handle));

    int i = 0;
    while (argv[i])
    {
        h->argv = realloc(h->argv, sizeof(char *) * (h->arg_index + 1));
        h->variable_index = 0;
        h->read_index = 0;

        scan_input(argv[i], h);

        h->arg_index += 1;
        i++;
    }
    h->argv = realloc(h->argv, sizeof(char *) * (h->arg_index + 1));
    h->argv[h->arg_index] = NULL;
    char **res = h->argv;

    i = 0;
    while (h->argv[i])
    {
        add_string_to_context(h->argv[i]);
        i++;
    }

    free(h);
    return res;
}

// s_e_v adapted for assignmets
void scan_expand_assignments(struct ast_redir_handle *hredir)
{
    struct expansion_handle *h = calloc(1, sizeof(struct expansion_handle));

    for (int i = 0; i < hredir->dict_size; i++)
    {
        h->argv = realloc(h->argv, sizeof(char *) * (h->arg_index + 1));
        h->variable_index = 0;
        h->read_index = 0;
        scan_input(hredir->assignment_dict[i].value, h);
        hredir->assignment_dict[i].value = h->argv[i];
        add_string_to_context(h->argv[i]);
        h->arg_index += 1;
    }
    if (h->argv)
    {
        free(h->argv);
    }
    free(h);
}

// s_e_v adapted for redir files
void scan_expand_redirs(struct ast_redir_handle *hredir)
{
    struct expansion_handle *h = calloc(1, sizeof(struct expansion_handle));

    for (int i = 0; i < hredir->size; i++)
    {
        h->argv = realloc(h->argv, sizeof(char *) * (h->arg_index + 1));
        h->variable_index = 0;
        h->read_index = 0;
        scan_input(hredir->redir[i].filename, h);
        hredir->redir[i].filename = h->argv[i];
        add_string_to_context(h->argv[i]);
        h->arg_index += 1;
    }
    if (h->argv)
    {
        free(h->argv);
    }
    free(h);
}

static int get_var_size(char *input, struct expansion_handle *h)
{
    int i = h->read_index;
    int size = 1;
    if (isdigit(input[i]) || input[i] == '@' || input[i] == '#'
        || input[i] == '?' || input[i] == '$' || input[i] == '*')
    {
        size++;
    }
    else if (input[i] == '{')
    {
        i++;
        size++;
        while (input[i] != '}' && input[i] != '\0')
        {
            if (!isalnum(input[i]) && input[i] != '_')
            {
                err(2, "expansion error : bad substitution");
            }
            i++;
            size++;
        }
        if (input[i] != '}')
        {
            err(2, "expansion error : unclosed bracket");
        }
        size++; // compter la closing bracket ${ ->}
    }
    else
    {
        while (!is_terminal_component(input[i]))
        {
            i++;
            size++;
        }
    }
    return size;
}

static void handle_single_quotes_exp(char *input, struct expansion_handle *h,
                                     int *max_size)
{
    char current = input[h->read_index++];

    while (current != '\'')
    {
        if (h->arg_size + 1 >= *max_size)
        {
            *max_size *= 2;
            h->argv[h->arg_index] =
                realloc(h->argv[h->arg_index], *max_size * sizeof(char));
        }
        if (current == '\0')
        {
            err(2, "expansion error: missing closing quote\n");
        }
        h->argv[h->arg_index][h->variable_index] = current;
        h->variable_index++;
        current = input[h->read_index++];
        h->arg_size++;
    }
}

static void handle_double_quotes_exp(char *input, struct expansion_handle *h,
                                     int *max_size)
{
    int flag_escaping = 0;
    char current = input[h->read_index++];
    while (current != '\"' || flag_escaping == 1)
    {
        if (h->arg_size + 1 >= *max_size)
        {
            *max_size *= 2;
            h->argv[h->arg_index] =
                realloc(h->argv[h->arg_index], *max_size * sizeof(char));
        }
        if (flag_escaping)
        {
            flag_escaping = 0;
            if (current != '$' && current != '\"')
            {
                h->argv[h->arg_index][h->variable_index] = '\\';
                h->variable_index++;
            }
            h->argv[h->arg_index][h->variable_index] = current;
            h->variable_index++;
            current = input[h->read_index++];
        }
        else if (current == '$')
        {
            int size = get_var_size(input, h);
            h->read_index--;
            expand_var(h, input + h->read_index, size);
            h->read_index += size;
            current = input[h->read_index++];
        }
        else if (current == '\\')
        {
            if (input[h->read_index] == '\0')
            {
                err(2, "expansion error : unexpected escape character\n");
            }
            else
            {
                flag_escaping = 1;
                current = input[h->read_index++];
            }
        }
        else
        {
            h->argv[h->arg_index][h->variable_index] = current;
            h->variable_index++;
            current = input[h->read_index++];
        }
        h->arg_size++;
    }
}

static void handle_quotes_exp(char *input, struct expansion_handle *h,
                              int *max_size, char current)
{
    if (current == '\'')
    {
        handle_single_quotes_exp(input, h, max_size);
    }
    else
    {
        handle_double_quotes_exp(input, h, max_size);
    }
}

static void handle_substitution(char *input, struct expansion_handle *h,
                                char *current, int *max_size)
{
    if (*current == '`' || input[h->read_index] == '(')
    {
        if (*current != '`')
        {
            h->read_index++;
        }
        command_substitution(input, h, current, max_size);
    }
    else
    {
        int size = get_var_size(input, h);
        if (size > 1)
        {
            h->read_index--;
            expand_var(h, input + h->read_index, size);
            h->read_index += size;
        }
        else
        {
            h->argv[h->arg_index][h->variable_index++] = *current;
        }
    }
    *current = input[h->read_index++];
}

void scan_input(char *input, struct expansion_handle *h)
{
    h->arg_size = 0;
    int max_size = 4;
    int flag_escaping = 0;
    char current = input[h->read_index];
    h->argv[h->arg_index] = calloc(max_size, sizeof(char));
    current = input[h->read_index++];
    while (current != '\0')
    {
        if (h->arg_size + 1 >= max_size)
        {
            max_size *= 2;
            h->argv[h->arg_index] =
                realloc(h->argv[h->arg_index], max_size * sizeof(char));
        }
        if (flag_escaping)
        {
            flag_escaping = 0;
            h->argv[h->arg_index][h->variable_index] = current;
            h->variable_index++;
            current = input[h->read_index++];
        }
        else if (current == '$' || current == '`')
        {
            handle_substitution(input, h, &current, &max_size);
        }
        else if (current == '\\')
        {
            if (input[h->read_index] == '\0')
            {
                err(2, "expansion error : unexpected escape character\n");
            }
            else
            {
                flag_escaping = 1;
                current = input[h->read_index++];
            }
        }
        else if (current == '\'' || current == '\"')
        {
            handle_quotes_exp(input, h, &max_size, current);
            current = input[h->read_index++];
        }
        else
        {
            h->argv[h->arg_index][h->variable_index++] = current;
            current = input[h->read_index++];
        }
        h->arg_size++;
    }
    h->argv[h->arg_index][h->variable_index] = 0;
}

// -- COMMAND SUBSTUTION UTILS --

static void save_sub_string(char *to_substitute, struct expansion_handle *h,
                            int *max_size_e, size_t i)
{
    to_substitute[i - 1] = '\0';
    char *parsed_sub = expand_cmd_sub(to_substitute);
    free(to_substitute);
    i = 0;
    char current = parsed_sub[i++];
    while (current != '\0')
    {
        if (h->arg_size + 1 >= *max_size_e)
        {
            *max_size_e *= 2;
            h->argv[h->arg_index] =
                realloc(h->argv[h->arg_index], *max_size_e * sizeof(char));
        }
        h->argv[h->arg_index][h->variable_index++] = current;
        current = parsed_sub[i++];
        h->arg_size++;
    }
    h->read_index--;
}

// builds the string (slightly long bc of recursion possibilities)
void command_substitution(char *input, struct expansion_handle *h,
                          char *current, int *max_size_e)
{
    int flag_escaping = 0;
    char delimiter = '`';
    if (*current == '$')
    {
        delimiter = ')';
    }
    size_t max_size = 4;
    char *to_substitute = calloc(max_size, sizeof(char));
    *current = input[h->read_index++];
    size_t i = 0;
    size_t nb_to_seek = 1; // $( ... $( ... ) ... ) => we will seek 1 more ')'
    while (nb_to_seek != 0)
    {
        if (i + 1 >= max_size)
        {
            max_size *= 2;
            to_substitute = realloc(to_substitute, max_size * sizeof(char));
        }
        if (flag_escaping)
        {
            flag_escaping = 0;
            to_substitute[i] = *current;
            *current = input[h->read_index++];
        }
        else if (*current == '\\')
        {
            if (input[h->read_index] == '\0')
            {
                err(2, "substitution error: unexpected escape \n");
            }
            else
            {
                to_substitute[i] = *current;
                flag_escaping = 1;
                *current = input[h->read_index++];
            }
        }
        else if (*current == delimiter)
        {
            nb_to_seek--;
            if (nb_to_seek != 0) // we need to keep all intermediate ')'
            {
                to_substitute[i] = *current;
            }
            *current = input[h->read_index++];
        }
        else if (*current == '$' && input[h->read_index] == '('
                 && delimiter == ')')
        {
            to_substitute[i] = *current; // we write '$' now, and '(' next loop
            nb_to_seek++;
            *current = input[h->read_index++];
        }
        else
        {
            to_substitute[i] = *current;
            *current = input[h->read_index++];
        }
        i++;
    }
    save_sub_string(to_substitute, h, max_size_e, i);
}
