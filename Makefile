SHELL := /bin/sh

PY := python3
PIP := $(PY) -m pip

CURDIR := $(shell pwd)

DOCS_DIR := $(CURDIR)/apps/panacee-pdf-generator
VENV_PY := $(DOCS_DIR)/.venv/bin/python

GEN_DIR := $(CURDIR)/apps/panacee-genetics
GEN_BIN := $(GEN_DIR)/panacee

DATA_DIR := $(CURDIR)/data
DATA_OUTPUT_DIR := $(DATA_DIR)/output


all: init build run

help:
	@echo "Top-level Makefile - available targets:"
	@echo "  init   - install Python dependencies"
	@echo "  build  - compile the C application (requires libMLV)"
	@echo "  run    - build then run the C binary and the Python app"
	@echo "  clean  - remove build artifacts and Python bytecode"
	@echo "  fclean - full clean including binaries"
	@echo "  re     - fclean then build"

init:
	@if [ ! -x "$(VENV_PY)" ]; then \
		echo "==> Creating virtualenv in $(DOCS_DIR)/.venv"; \
		python3 -m venv "$(DOCS_DIR)/.venv"; \
		echo "==> Installing Python dependencies"; \
		"$(DOCS_DIR)/.venv/bin/pip" install "$(DOCS_DIR)"; \
	else \
		echo "==> Virtualenv already present at $(DOCS_DIR)/.venv (skipping)"; \
	fi

build:
	@echo "==> Building C application in $(GEN_DIR)"
	@$(MAKE) -C $(GEN_DIR) all

run: init build
	@mkdir -p "$(DATA_OUTPUT_DIR)"
	@echo "==> Starting C binary"
	@cd $(GEN_DIR) && ./panacee && \
	echo "==> Starting Python application" && \
	$(VENV_PY) "$(DOCS_DIR)/src/main.py"

clean:
	@echo "==> Cleaning C build and Python bytecode"
	@$(MAKE) -C $(GEN_DIR) clean || true

fclean: clean
	@echo "==> Full clean: removing binaries"
	@$(MAKE) -C $(GEN_DIR) fclean || true
	@rm -f $(GEN_BIN) 2>/dev/null || true

re: fclean build

.PHONY: all help init build run clean fclean re
