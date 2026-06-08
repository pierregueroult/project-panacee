CC = cc
CFLAGS = -Wall -ansi -g
LDFLAGS =

# --- MACOS FORMAT
# CLANG_FORMAT=/opt/homebrew/opt/llvm/bin/clang-format
# --- LINUX FORMAT
CLANG_FORMAT ?= clang-format
DOXYGEN ?= doxygen

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

docs:
	$(DOXYGEN) Doxyfile

clean:
	rm -rf $(OBJ_DIR)
	rm -rf bin/docs

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all run format docs clean fclean re
