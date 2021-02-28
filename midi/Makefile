CC = gcc
CFLAGS = -Wall -g -Werror -pedantic

wsrc = ./filereader.c\
       ./filewriter.c\
       ./entry.c\
       ./../wfc/wfc.c

wobj = $(wsrc:.c=.o)

all: entry

entry: $(wobj)
	$(CC) -o $@ $^ $(CFLAGS)


