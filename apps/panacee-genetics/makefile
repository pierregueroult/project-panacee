CC = cc
CFLAGS = -Wall -ansi -g
LDFLAGS =

# Override if clang-format is not on PATH, e.g. on macOS with brew's LLVM:
#   make format CLANG_FORMAT=$(brew --prefix llvm)/bin/clang-format
CLANG_FORMAT ?= clang-format

MLV_CFLAGS = $(shell pkg-config --cflags MLV)
MLV_LIBS = $(shell pkg-config --libs MLV)

NAME    = panacee
SRC_DIR = src
OBJ_DIR = bin/obj

SRC = $(shell find $(SRC_DIR) -type f -name "*.c")
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $(NAME) $(MLV_LIBS) -lm $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(MLV_CFLAGS) -c $< -o $@

run: $(NAME)
	./$(NAME)

format:
	$(CLANG_FORMAT) -i $(shell find $(SRC_DIR) -type f \( -name "*.c" -o -name "*.h" \))

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all run format clean fclean re
