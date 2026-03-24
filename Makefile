SHELL := /bin/sh

PY := python3
PIP := $(PY) -m pip

CURDIR := $(shell pwd)

DOCS_DIR := $(CURDIR)/apps/panacee-documents
DOCS_REQ := $(DOCS_DIR)/requirement.txt

GEN_DIR := $(CURDIR)/apps/panacee-genetics
GEN_BIN := $(GEN_DIR)/panacee


all: init build run

help:
	@echo "Top-level Makefile - available targets:"
	@echo "  init   - install Python dependencies"
	@echo "  build  - compile the C application"
	@echo "  run    - build then run the C binary (background) and the Python app (foreground)"
	@echo "  clean  - remove build artifacts and Python bytecode"
	@echo "  fclean - full clean including binaries"
	@echo "  re     - fclean then build"

init:
	@echo "==> Installing Python dependencies from $(DOCS_REQ)"
	@$(PIP) install -r "$(DOCS_REQ)"

build:
	@echo "==> Building C application in $(GEN_DIR)"
	@$(MAKE) -C $(GEN_DIR) all

run: build
	@echo "==> Starting C binary"
	@cd $(GEN_DIR) && ./panacee; \
	echo "==> Starting Python application"; \
	$(PY) "$(DOCS_DIR)/main.py"; \

clean:
	@echo "==> Cleaning C build and Python bytecode"
	@$(MAKE) -C $(GEN_DIR) clean || true

fclean: clean
	@echo "==> Full clean: removing binaries"
	@$(MAKE) -C $(GEN_DIR) fclean || true
	@rm -f $(GEN_BIN) 2>/dev/null || true

re: fclean build

.PHONY: all help init build run clean fclean re
