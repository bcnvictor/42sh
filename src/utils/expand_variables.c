#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "context.h"
#include "expansion.h"
#include "io_backend/handle_input.h"
#include "parser/parser.h"

char *expand_cmd_sub(char *subcmd)
{
    struct context *ctx = get_context();
    free(ctx->buffer);
    char *argv[] = { "mdsub", "-c", subcmd };
    FILE *input = getinput(3, argv);
    struct lexer *lexer = lexer_new(input);
    struct ast_base *ast = parse_input(lexer);
    int tempsave = open(".cmdsub", O_CREAT | O_TRUNC | O_WRONLY, 0644);
    int saveout = dup(STDOUT_FILENO);
    dup2(tempsave, STDOUT_FILENO);
    // sub command execution
    run_ast(ast);
    free_ast(ast);
    fclose(input);
    lexer_free(lexer);
    close(tempsave);
    dup2(saveout, STDOUT_FILENO);
    close(saveout);
    // Getting output from file
    FILE *saved_output = fopen(".cmdsub", "r+");
    fseek(saved_output, 0, SEEK_END);
    size_t fsize = ftell(saved_output);
    rewind(saved_output);
    char *res = malloc(fsize + 1);
    fread(res, 1, fsize, saved_output);
    res[fsize - 1] = 0;
    size_t index = 0;
    while (res[index] != 0)
    {
        if (res[index] == '\n')
        {
            res[index] = ' ';
        }
        index++;
    }
    fclose(saved_output);
    add_string_to_context(res);
    return res;
}
void expand_var(struct expansion_handle *exp, char *var, int var_length)
{
    if (var_length < 2)
        errx(1, "expansion: var_length must be greater than 2");

    int offset = 1;
    var_length--;

    if (var[offset] == '{')
    {
        offset++;
        var_length -= 2;
    }
    int index = exp->variable_index;
    char *var_value = get_variable(var + offset, var_length, exp);
    if (var_value[0] == 0)
        return;
    char *cur = exp->argv[exp->arg_index];
    if (exp->arg_size < exp->variable_index + 1)
    {
        cur = realloc(cur, exp->variable_index + 1);
        exp->arg_size = exp->variable_index + 1;
    }
    if (!strcpy(cur + index, var_value))
        errx(1, "expansion: Unable to use strcpy");
    exp->argv[exp->arg_index] = cur;
}
