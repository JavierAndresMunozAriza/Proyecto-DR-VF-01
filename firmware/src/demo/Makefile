#
# Makefile for the demo route and its host test
#
# `make test` builds and runs the unit tests against the real geofence
# engine; `make clean` removes artifacts. Override the compiler with
# `make CC=clang`.

ifeq ($(origin CC),default)
CC = gcc
endif
CFLAGS = -Wall -Wextra -std=c99 -O2 -I../../include
LDFLAGS = -lm

TARGET = demo_route_test
SOURCES = demo_route.c test_demo_route.c ../fence/fence_square.c ../geofence/geofence.c
HEADERS = ../../include/demo_route.h ../../include/fence_square.h ../../include/geofence.h ../../include/geo_constants.h

.PHONY: test clean

test: $(TARGET)
	./$(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)
