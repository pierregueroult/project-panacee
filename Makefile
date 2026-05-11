SHELL := /bin/sh

PY := python3
PIP := $(PY) -m pip

CURDIR := $(shell pwd)

DOCS_DIR := $(CURDIR)/apps/panacee-pdf-generator
VENV_PY := $(DOCS_DIR)/.venv/bin/python

GEN_DIR := $(CURDIR)/apps/panacee-genetics
GEN_BIN := $(GEN_DIR)/panacee
GEN_BIN_NO_MLV := $(GEN_DIR)/panacee-no-mlv


all: init build run

help:
	@echo "Top-level Makefile - available targets:"
	@echo "  init         - install Python dependencies"
	@echo "  build        - compile the C application (requires lib_mlv)"
	@echo "  build-no-mlv - compile the C application without lib_mlv"
	@echo "  run          - build then run the C binary and the Python app"
	@echo "  run-no-mlv   - build-no-mlv then run without lib_mlv"
	@echo "  clean        - remove build artifacts and Python bytecode"
	@echo "  fclean       - full clean including binaries"
	@echo "  re           - fclean then build"

init:
	@echo "==> Creating virtualenv in $(DOCS_DIR)/.venv"
	@python3 -m venv "$(DOCS_DIR)/.venv"
	@echo "==> Installing Python dependencies"
	@"$(DOCS_DIR)/.venv/bin/pip" install "$(DOCS_DIR)"

build:
	@echo "==> Building C application in $(GEN_DIR)"
	@$(MAKE) -C $(GEN_DIR) all

build-no-mlv:
	@echo "==> Building C application without lib_mlv in $(GEN_DIR)"
	@$(MAKE) -C $(GEN_DIR) no-mlv

run: build
	@echo "==> Starting C binary"
	@cd $(GEN_DIR) && ./panacee; \
	echo "==> Starting Python application"; \
	$(VENV_PY) "$(DOCS_DIR)/src/main.py"; \

run-no-mlv: build-no-mlv
	@echo "==> Starting C binary (no-mlv)"
	@cd $(GEN_DIR) && ./panacee-no-mlv; \
	echo "==> Starting Python application"; \
	$(PY) "$(DOCS_DIR)/main.py"; \

clean:
	@echo "==> Cleaning C build and Python bytecode"
	@$(MAKE) -C $(GEN_DIR) clean || true

fclean: clean
	@echo "==> Full clean: removing binaries"
	@$(MAKE) -C $(GEN_DIR) fclean || true
	@rm -f $(GEN_BIN) $(GEN_BIN_NO_MLV) 2>/dev/null || true

re: fclean build

.PHONY: all help init build build-no-mlv run run-no-mlv clean fclean re
