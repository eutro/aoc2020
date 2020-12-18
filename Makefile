SRC_DIR := ./src
OBJ_DIR := ./obj
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

LSP_DIR := ./lisp
LSP_OBJ_DIR := ./lisp/obj
LSP_FILES := $(wildcard $(LSP_DIR)/*.c)
LSP_OBJS := $(patsubst $(LSP_DIR)/%.c,$(LSP_OBJ_DIR)/%.o,$(LSP_FILES))

aoc: $(OBJ_FILES) $(LSP_OBJS)
	$(CC) -o aoc $(OBJ_FILES) $(LSP_OBJS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(OBJ_DIR)
	$(CC) -o $@ -c $(CFLAGS) $<

$(LSP_OBJ_DIR)/%.o: $(LSP_DIR)/%.c $(LSP_OBJ_DIR)
	$(CC) -o $@ -c $(CFLAGS) $<

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(LSP_OBJ_DIR):
	mkdir $(LSP_OBJ_DIR)
