CC = cc
CFLAGS = -Wall -ansi -g
LDFLAGS =

# --- MACOS FORMAT
# CLANG_FORMAT=/opt/homebrew/opt/llvm/bin/clang-format
# --- LINUX FORMAT
CLANG_FORMAT ?= clang-format
DOXYGEN ?= doxygen

ifeq ($(shell uname -s),Darwin)
PKG_CONFIG_ENV := PKG_CONFIG_PATH=$(HOME)/.local/mlv-deps/lib/pkgconfig:$(shell brew --prefix)/lib/pkgconfig:$(shell brew --prefix)/opt/sdl12-compat/lib/pkgconfig
endif
MLV_CFLAGS = $(shell $(PKG_CONFIG_ENV) pkg-config --cflags MLV)
MLV_LIBS   = $(shell $(PKG_CONFIG_ENV) pkg-config --libs MLV)

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
