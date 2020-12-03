SRC_DIR := ./src
OBJ_DIR := ./obj
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

aoc: $(OBJ_FILES)
	cc -o aoc $(OBJ_FILES)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(OBJ_DIR)
	cc -o $@ -c $<

$(OBJ_DIR):
	mkdir $(OBJ_DIR)
