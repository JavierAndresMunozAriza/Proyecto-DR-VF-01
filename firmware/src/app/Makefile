#
# Makefile for the application state machine and its host test
#
# `make test` builds and runs the unit tests; `make clean` removes artifacts.
# Override the compiler with `make CC=clang`.

ifeq ($(origin CC),default)
CC = gcc
endif
CFLAGS = -Wall -Wextra -std=c99 -O2 -I../../include

TARGET = app_fsm_test

.PHONY: test clean

test: $(TARGET)
	./$(TARGET)

$(TARGET): app_fsm.c test_app_fsm.c ../../include/app_fsm.h
	$(CC) $(CFLAGS) app_fsm.c test_app_fsm.c -o $@

clean:
	rm -f $(TARGET)
