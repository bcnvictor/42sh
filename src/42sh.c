#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>

#include "ast/ast.h"
#include "io_backend/handle_input.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "utils/context.h"
#include "utils/expansion.h"

int sh_main(int argc, char *argv[])
{
    FILE *input = getinput(argc, argv);
    struct lexer *lexer = lexer_new(input);
    int i = 0;
    char buf[9];
    sprintf(buf, "ast0.dot");
    while (lexer_peek(lexer).type != TOKEN_EOFI)
    {
            struct ast_base *ast = parse_input(lexer);
        run_ast(ast);
        free_ast(ast);
        buf[3] = ++i + '0';
        (void)i;
    }
    struct context *ctx = get_context();

    fclose(input);
    lexer_free(lexer);
    return ctx->return_code;
}

int main(int argc, char *argv[])
{
    struct context *ctx = init_context();
    ctx->args = argv;
    ctx->nb_args = argc;
    if (!ctx)
    {
        perror("42sh: failed to initialize context");
        return 1;
    }
    int ret = sh_main(argc, argv);
    free_context(ctx);
    return ret;
}
