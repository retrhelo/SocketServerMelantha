# Author: Artyom Liu 

TARGET=melantha

CC=gcc

# Compiler flags 
C_FLAGS=-g -Iinc -Wall

# Linker flags 
LINK_FLAGS=-g -pthread

SRC= \
src/config.c \
src/fifo.c \
src/http.c \
src/mthread.c \
src/main.c 

OBJ= \
obj/config.o \
obj/fifo.o \
obj/http.o \
obj/mthread.o \
obj/main.o 

build/$(TARGET): $(OBJ) build 
	$(CC) $(LINK_FLAGS) -o $@ $(OBJ) 

obj/config.o: src/config.c obj 
	$(CC) $(C_FLAGS) -c $< -o $@
obj/fifo.o: src/fifo.c obj 
	$(CC) $(C_FLAGS) -c $< -o $@
obj/http.o: src/http.c obj 
	$(CC) $(C_FLAGS) -c $< -o $@
obj/mthread.o: src/mthread.c obj 
	$(CC) $(C_FLAGS) -c $< -o $@
obj/main.o: src/main.c obj 
	$(CC) $(C_FLAGS) -c $< -o $@

build: 
	mkdir build 

obj: 
	mkdir obj 

clean: 
	rm -r obj build 
