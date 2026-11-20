#
# Makefile for the square fence builder and its host test
#
# `make test` builds and runs the unit tests against the real geofence
# engine; `make clean` removes artifacts. Override the compiler with
# `make CC=clang`.

ifeq ($(origin CC),default)
CC = gcc
endif
CFLAGS = -Wall -Wextra -std=c99 -O2 -I../../include
LDFLAGS = -lm

TARGET = fence_square_test
SOURCES = fence_square.c test_fence_square.c ../geofence/geofence.c

.PHONY: test clean

test: $(TARGET)
	./$(TARGET)

$(TARGET): $(SOURCES) ../../include/fence_square.h ../../include/geofence.h
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)
