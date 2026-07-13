## pour tout clean >> find . '!' '(' -name "*.sh" -o -name "*.am" -o -name "*.c" -o -name "*.h" -o -name "*.ac" ')' -type f -delete

AC_INIT([42sh], [1.0], [42sh@assistants.epita.fr])

AM_INIT_AUTOMAKE([subdir-objects] [foreign])

AM_SILENT_RULES([yes])

AM_PROG_AR

AC_PROG_RANLIB

AC_PROG_CC

#AX_COMPILER_FLAGS([], [], [], [-Wall -Wextra -Werror -Wvla -pedantic -std=c99])

AC_CONFIG_FILES([
                 Makefile
                 src/Makefile
                 src/ast/Makefile
                 src/parser/Makefile
                 src/lexer/Makefile
                 src/io_backend/Makefile
                 src/builtins/Makefile
                 src/utils/Makefile
                 tests/Makefile
                 ])

AC_OUTPUT
