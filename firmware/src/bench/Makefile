#
# Makefile for the benchmark polygon generator and its host test
#
# `make test` builds and runs the unit tests against the real geofence
# engine; `make clean` removes artifacts. Override the compiler with
# `make CC=clang`.

ifeq ($(origin CC),default)
CC = gcc
endif
CFLAGS = -Wall -Wextra -std=c99 -O2 -I../../include
LDFLAGS = -lm

TARGET = bench_polygon_test
SOURCES = bench_polygon.c test_bench_polygon.c ../geofence/geofence.c
HEADERS = ../../include/bench_polygon.h ../../include/geofence.h ../../include/geo_constants.h

.PHONY: test clean

test: $(TARGET)
	./$(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)
