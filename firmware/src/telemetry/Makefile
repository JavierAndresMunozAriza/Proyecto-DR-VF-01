#
# Makefile for the telemetry report and its host test
#
# `make test` builds and runs the unit tests; `make clean` removes artifacts.
# Override the compiler with `make CC=clang`.

ifeq ($(origin CC),default)
CC = gcc
endif
CFLAGS = -Wall -Wextra -std=c99 -O2 -I../../include
LDFLAGS = -lm

TARGET = telemetry_test
SOURCES = telemetry.c test_telemetry.c ../app/app_fsm.c

.PHONY: test clean

test: $(TARGET)
	./$(TARGET)

$(TARGET): $(SOURCES) ../../include/telemetry.h ../../include/app_fsm.h
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)
