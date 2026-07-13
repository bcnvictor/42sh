#include "ast.h"

#include <err.h>
#include <utils/myutils.h>

const char *ast_repr(struct ast_base *node)
{
    static const char *const names[] = {
#define XAST_F(Enum, Name) [AST_##Enum] = #Enum,
        XAST(XAST_F)
#undef XAST_F
    };
    if (node->type == AST_CMD)
    {
        return (((struct ast_simple_command *)node)->args[0]);
    }
    return names[node->type];
}
