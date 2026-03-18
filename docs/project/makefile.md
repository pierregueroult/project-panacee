# Makefile notes 

## 1 Base

- one executable : `panacee`
- sources are in `src/`
- object files are in `build/`

## 2 Variables

- `CC = cc`: compiler
- `CFLAGS = -Wall -ansi`: best flags for real
- `NAME = panacee`: output program name
- `SRC = $(shell find src -type f -name "*.c")`: this finds all `.c` files in `src/` and subfolders
- `OBJ = $(patsubst src/%.c,build/%.o,$(SRC))`: converts source paths to object paths; example: `src/somefoler/somefile.c` -> `build/somefoler/somefile.o`; see: [patsubt doc](https://www.gnu.org/software/make/manual/html_node/Text-Functions.html)

## 3) Targets

- `all`: default target, builds the program
- `$(NAME)`: links all `.o` files to create `panacee`
- `build/%.o: src/%.c`: generic rule to compile any source file into `build/` we also create the folder before compiling: `mkdir -p $(dir $@)`
- `run`: runs `./panacee`
- `clean`: removes files in `build/` but keeps `.gitkeep`
- `fclean`: does `clean` + removes the executable
- `re`: full rebuild (`fclean` then `all`)
