# Makefile for LXPKG
CC = gcc
CFLAGS = -shared -fPIC -Wall -O2
LIBS = -larchive
TARGET = liblxpkg.so
SRC = liblxpkg.c
PYTHON = ./venv/bin/python
PIP = ./venv/bin/pip
SCRIPT = lxpkg.py

# Default target
all: $(TARGET)

# Build the C shared library
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

# Create virtual environment and install dependencies
deps:
	sudo apt install libarchive-dev python3-pip python3-venv
	python3 -m venv venv
	$(PIP) install rich toml

# Clean up
clean:
	rm -f $(TARGET)
	rm -rf venv

# Run the Python script with a test command
test: $(TARGET)
	$(PYTHON) $(SCRIPT) install busybox --dry-run

.PHONY: all clean test deps
