CC = gcc
CFLAGS = -std=c99 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -Wall -Wextra -Werror -Wno-unused-parameter -fno-asm -g

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

# Find all .c files in src/ and map them to .o files in obj/
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TARGET = shell.out

all: $(TARGET)

# Link object files to create the final binary
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compile .c files to .o files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Ensure the object directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)