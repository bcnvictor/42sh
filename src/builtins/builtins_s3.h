#ifndef BUILTINS_S3_H
#define BUILTINS_S3_H

#include "utils/myutils.h"

int cd_builtin(char **argv);
int exit_builtin(char **argv);
int export_builtin(char **argv);
int continue_builtin(char **argv);
int break_builtin(char **argv);
int dot_builtin(char **argv);
int unset_builtin(char **argv);
int alias_builtin(char **argv);
int unalias_builtin(char **argv);

#endif /* !BUILTINS_S3_H */
