# Author: Artyom Liu 

TARGET := melantha

CC := gcc

# Compiler flags 
C_FLAGS := -g -Iinc -Wall -c 

# Linker flags 
LINK_FLAGS := -g -pthread

SRC := \
src/config.c \
src/fifo.c \
src/http.c \
src/mthread.c \
src/main.c 

OBJ := $(addprefix obj/, $(notdir $(SRC:.c=.o)))

.PHONY: all
all: $(OBJ) build 
	$(CC) $(LINK_FLAGS) -o build/$(TARGET) $(OBJ) 

obj/%.o: src/%.c obj
	$(CC) $(C_FLAGS) $< -o $@

build: 
	mkdir build 

obj: 
	mkdir obj 

.PHONY: clean
clean: 
	rm -r obj build 
