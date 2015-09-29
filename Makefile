#!/bin/bash
#file: MakeFile
#*****************************************
#Macro Variables CC and CFLAGS are created
#*****************************************
CC=gcc
CFLAGS = -c -w -g

#Format of commands in Make file is as follows
#*********************************************
#Target: Dependency
#<----tab---> <command>
#*********************************************
#***************************************************
#command for creation of executable file is:
#command = gcc <filename.c> -o <executable_filename>
#***************************************************
all: prog1 prog2

prog1: server.o
	$(CC) server.o -o server/server
prog2: client.o
	$(CC) client.o -o client/client
#***********************************************************
#other command will be of the following form:
#command = gcc [options] <filename.c> [optional header files]
#their purpose is to compile each
#source code files in the program
#***********************************************************
main.o: main.c
	$(CC) $(CFLAGS) main.c
	
server.o: server/server.c
	$(CC) $(CFLAGS) server/server.c
	
client.o: client/client.c
	$(CC) $(CFLAGS) client/client.c
#*********************************************************************
#clean is used to destroy all executable files
#so that the next make will compile all source code files from scratch
#*********************
#************************************************
clean:
	rm -rf *.o prog
