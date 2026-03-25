# Makefile notes

## 1) Overview

This project uses **two Makefiles**:

- `makefile` (project root): global orchestration for Python + C
- `apps/panacee-genetics/makefile`: C genetic engine build

## 2) Root Makefile (`/makefile`)

- `all`: runs `init build run`
- `help`: prints available targets
- `init`: installs Python dependencies from `apps/panacee-documents/requirement.txt`
- `build`: runs `make -C apps/panacee-genetics all`
- `run`: depends on `build`, then:
  1. runs the C binary `./panacee` inside `apps/panacee-genetics`
  2. runs the Python app `apps/panacee-documents/main.py`
- `clean`: cleans C build artifacts through the genetics makefile (`clean`)
- `fclean`: runs `clean`, then performs full cleanup 
- `re`: runs `fclean` then `build`

## 3) Genetics Makefile (`apps/panacee-genetics/makefile`)

- `all`: builds `$(NAME)`
- `$(NAME)`: links all object files `$(OBJ)` into `panacee`
- `bin/%.o: src/%.c`: generic compile rule
  - creates output directory with `mkdir -p $(dir $@)`
  - compiles with `$(CC) $(CFLAGS) -c $< -o $@`
- `run`: runs `./panacee`
- `clean`: removes `bin/` contents while keeping `.gitkeep`
- `fclean`: runs `clean` then removes `panacee`
- `re`: runs `fclean` then `all`
