# @file Makefile battleship (for server and client)
# @author Hannes Brantner Matrikelnummer: 01614466
# @date 18.10.2017
# @brief all the compilation and linking work is done automatically by typing make in the console
# @details the common.o linking works as expected now

CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean

all: server client

%: %.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

server.o: server.c common.h 
client.o: client.c common.h
common.o: common.c common.h
server: server.o common.o 
client: client.o common.o 

clean:
	rm -rf *.o server client