CC 			= gcc
CFLAGS  = -Wall -g -D_POSIX_SOURCE -D_DEFAULT_SOURCE -std=c99 -Werror -pedantic -lm


src =  $(wildcard *.c)

tests.out: $(src)
	$(CC) -o $@ $^ $(CFLAGS)
