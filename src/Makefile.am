SUBDIRS = ast \
		  lexer \
		  parser \
		  io_backend \
		  utils \
		  builtins


bin_PROGRAMS = 42sh

42sh_SOURCES = 42sh.c

42sh_CPPFLAGS = -I%D%

42sh_CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla -pedantic -D_DEFAULT_SOURCE -fsanitize=address

42sh_LDADD = ast/libast.a parser/libparser.a \
	lexer/liblexer.a io_backend/libbackend.a \
	builtins/libbuiltins.a utils/libmyutils.a
