#ifndef HANDLE_INPUT_H
#define HANDLE_INPUT_H

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

FILE *getinput(int argc, char **argv /*, struct context *context*/);

char *handle_single_quotes(size_t *max_size, char *res, size_t *size, FILE *fd);

char *skip_spaces(FILE *fd, int *space_flag, char *res);

char *get_next_word(FILE *fd, int *space_flag);

#endif /* !HANDLE_INPUT_H */
