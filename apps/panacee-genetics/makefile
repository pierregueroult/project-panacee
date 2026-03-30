CC = cc
CFLAGS = -Wall -ansi -g
LDFLAGS = -lMLV -lm
NAME = panacee

SRC_DIR = src
OBJ_DIR = bin

SRC = $(shell find $(SRC_DIR) -type f -name "*.c")
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $(NAME) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(NAME)
	./$(NAME)

clean:
	find $(OBJ_DIR) -mindepth 1 ! -name ".gitkeep" -delete

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all run clean fclean re
