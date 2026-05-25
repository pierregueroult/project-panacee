CC = cc
CFLAGS = -Wall -ansi -g -DUSE_MLV
LDFLAGS =

# Linux (pkg-config standard) :
MLV_CFLAGS = $(shell pkg-config --cflags MLV)
MLV_LIBS = $(shell pkg-config --libs MLV)
# macOS (brew + sdl12-compat) :
# PKG_CONFIG_PATH := $(HOME)/.local/mlv-deps/lib/pkgconfig:$(shell brew --prefix)/lib/pkgconfig:$(shell brew --prefix)/opt/sdl12-compat/lib/pkgconfig
# MLV_CFLAGS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --cflags MLV)
# MLV_LIBS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs MLV)

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

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all run clean fclean re
