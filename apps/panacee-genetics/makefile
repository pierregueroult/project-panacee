CC = cc
CFLAGS = -Wall -ansi -g
NAME = panacee
NAME_NO_MLV = panacee-no-mlv

SRC_DIR = src
OBJ_DIR = bin/full
OBJ_DIR_NO_MLV = bin/no-mlv

SRC = $(shell find $(SRC_DIR) -type f -name "*.c")
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))
OBJ_NO_MLV = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR_NO_MLV)/%.o,$(SRC))

all: $(NAME)

$(NAME): CFLAGS += -DUSE_MLV
$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $(NAME) -lMLV -lm

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

no-mlv: $(NAME_NO_MLV)

$(NAME_NO_MLV): $(OBJ_NO_MLV)
	$(CC) $(OBJ_NO_MLV) -o $(NAME_NO_MLV) -lm

$(OBJ_DIR_NO_MLV)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(NAME)
	./$(NAME)

run-no-mlv: $(NAME_NO_MLV)
	./$(NAME_NO_MLV)

clean:
	-find $(OBJ_DIR) -mindepth 1 -delete 2>/dev/null; true
	-find $(OBJ_DIR_NO_MLV) -mindepth 1 -delete 2>/dev/null; true

fclean: clean
	rm -f $(NAME) $(NAME_NO_MLV)

re: fclean all

.PHONY: all no-mlv run run-no-mlv clean fclean re
