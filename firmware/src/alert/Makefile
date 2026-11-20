#
# Makefile for the alert color mapping and its host test
#
# `make test` builds and runs the unit tests; `make clean` removes artifacts.
# Override the compiler with `make CC=clang`.

ifeq ($(origin CC),default)
CC = gcc
endif
CFLAGS = -Wall -Wextra -std=c99 -O2 -I../../include
LDFLAGS = -lm

TARGET = alert_color_test

.PHONY: test clean

test: $(TARGET)
	./$(TARGET)

$(TARGET): alert_color.c test_alert_color.c ../../include/alert_color.h
	$(CC) $(CFLAGS) alert_color.c test_alert_color.c -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)
