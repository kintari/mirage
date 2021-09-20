CC=gcc
CFLAGS=-g -Wall -Werror -fPIC -D_POSIX_C_SOURCE=200809L

MAKEFILE:=$(CURDIR)/$(lastword $(MAKEFILE_LIST))

CONFIG_ENV=$(shell realpath $(dir $(MAKEFILE))/config.env)
include $(CONFIG_ENV)

SRC=src
CFILES=$(wildcard $(SRC)/*.c)
HFILES=$(wildcard $(SRC)/*.h)

TARGET=$(BIN)/mirage

$(TARGET): $(CFILES) $(HFILES) | $(BIN)
	$(CC) $(CFLAGS) -o $@ $(CFILES)

$(BIN):
	-mkdir -p $@

.PHONY: clean

clean:
	-rm $(TARGET)