#include "builtins_s1_2.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ast/ast.h"
#include "io_backend/handle_input.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "utils/context.h"
#include "utils/myutils.h"

int true_builtin(char **argv)
{
    (void)argv;
    return 0;
}

int false_builtin(char **argv)
{
    (void)argv;
    return 1;
}

static void print_with_escapes(const char *str)
{
    for (const char *p = str; *p != '\0'; p++)
    {
        if (*p == '\\')
        {
            p++;
            switch (*p)
            {
            case 'n':
                putchar('\n');
                break;
            case 't':
                putchar('\t');
                break;
            case '\\':
                putchar('\\');
                break;
            default:
                putchar('\\');
                putchar(*p);
                break;
            }
        }
        else
        {
            putchar(*p);
        }
    }
}

int echo_builtin(char **argv)
{
    bool newline = true; // Retour a la ligne
    bool interpret_escape = false; // Pour \n et \t

    int start = 1;
    int i = 0;
    for (; argv[start] != NULL;)
    {
        i = 0;
        bool verif = false;
        char *tmp = argv[start];
        if (!tmp)
            return 2;
        if (tmp[i] == '-')
        {
            i++;
            while (tmp[i] == 'n' || tmp[i] == 'e' || tmp[i] == 'E')
            {
                if (tmp[i] == 'n')
                {
                    newline = false;
                    verif = true;
                }
                else if (tmp[i] == 'e')
                {
                    interpret_escape = true;
                    verif = true;
                }
                else if (tmp[i] == 'E')
                {
                    interpret_escape = false;
                    verif = true;
                }
                i++;
            }
            if (verif)
                start++;
            else
                break;
        }
        else
            break;
    }
    for (int j = start; argv[j] != NULL; j++)
    {
        if (j > start)
            putchar(' ');
        if (interpret_escape)
        {
            print_with_escapes(argv[j]);
        }
        else
        {
            printf("%s", argv[j]);
        }
    }
    if (newline)
    {
        putchar('\n');
    }
    return 0;
}

int args_builtin(char **argv)
{
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("42sh: args_builtin: fork failed\n");
        return 1;
    }
    if (pid == 0)
    {
        execvp(argv[0], argv);
        perror("42sh: args_builtin: execvp failed");
        exit(127);
    }

    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    return 0;
}
