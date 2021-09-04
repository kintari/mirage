CC=gcc
CFLAGS=-g -Wall -Werror -O0 -fPIC -D_POSIX_C_SOURCE=200809L
PLATFORM=x86_64-linux-gnu
CONFIG=debug
BIN=build/$(PLATFORM)/$(CONFIG)
SRC=src
CFILES=$(wildcard $(SRC)/*.c)
HFILES=$(wildcard $(SRC)/*.h)

$(BIN)/mirage: $(CFILES) $(HFILES) | $(BIN)
	$(CC) $(CFLAGS) -o $@ $(CFILES)

$(BIN):
	-mkdir -p $@