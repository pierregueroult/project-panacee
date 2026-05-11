CC          = cc
CFLAGS = -Wall -ansi -g
NAME        = panacee
NAME_NO_MLV = panacee-no-mlv

SRC_DIR        = src
OBJ_DIR        = bin/full
OBJ_DIR_NO_MLV = bin/no-mlv
RESULTS_DIR    = $(SRC_DIR)/results

SRC        = $(shell find $(SRC_DIR) -type f -name "*.c")
OBJ        = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))
OBJ_NO_MLV = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR_NO_MLV)/%.o,$(SRC))

all: $(NAME)

# ---- main MLV build ----
$(NAME): CFLAGS += -DUSE_MLV
$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $(NAME) -lMLV -lm

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ---- headless (no MLV) build ----
no-mlv: $(NAME_NO_MLV)

$(NAME_NO_MLV): $(OBJ_NO_MLV)
	$(CC) $(OBJ_NO_MLV) -o $(NAME_NO_MLV) -lm

$(OBJ_DIR_NO_MLV)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ---- runners ----
$(RESULTS_DIR):
	@mkdir -p $(RESULTS_DIR)

run: $(NAME) $(RESULTS_DIR)
	./$(NAME)

run-no-mlv: $(NAME_NO_MLV) $(RESULTS_DIR)
	./$(NAME_NO_MLV)

# ---- cleanup ----
clean:
	rm -rf $(OBJ_DIR) $(OBJ_DIR_NO_MLV)

fclean: clean
	rm -f $(NAME) $(NAME_NO_MLV)

re: fclean all

.PHONY: all no-mlv run run-no-mlv clean fclean re
