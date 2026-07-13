lib_LIBRARIES = libmyutils.a

libmyutils_a_SOURCES =  myutils.c myutils.h context.h context.c expansion.c \
						expansion.h expand_variables.c

libmyutils_a_CPPFLAGS = -I$(top_srcdir)/src

libmyutils_a_CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla -pedantic -D_DEFAULT_SOURCE

noinst_LIBRARIES = libmyutils.a
