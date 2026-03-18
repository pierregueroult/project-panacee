CC = cc
CFLAGS = -Wall -ansi
NAME = panacee

SRC = $(shell find src -type f -name "*.c")
OBJ = $(patsubst src/%.c,build/%.o,$(SRC))

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $(NAME)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(NAME)
	./$(NAME)

clean:
	find build -mindepth 1 ! -name ".gitkeep" -delete

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all run clean fclean re
