#
# Makefile for the geofence engine and its host test
#
# Following Elecia White's principles:
# - Simple, clear build process
# - Separate compilation and linking
# - Easy testing integration
#
# `make` and `make test` build and run the single test binary,
# test_geofence.c. Override the compiler with `make CC=clang`.

# Compiler settings. make predefines CC as "cc", so a plain `CC ?= gcc`
# would never apply; keep the default gcc but honor CC from the
# environment or the command line.
ifeq ($(origin CC),default)
CC = gcc
endif
CFLAGS = -Wall -Wextra -std=c99 -O2 -I../../include
LDFLAGS = -lm

# Source files
SRCDIR = .
INCDIR = ../../include
TESTDIR = .

SOURCES = $(SRCDIR)/geofence.c
HEADERS = $(INCDIR)/geofence.h
TEST_SOURCES = $(TESTDIR)/test_geofence.c

# Object files
OBJECTS = $(SOURCES:.c=.o)
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)

# Targets
TARGET = geofence_test
EXECUTABLE = $(TARGET)
DEBUG_EXECUTABLE = $(TARGET)_debug

.PHONY: all test clean debug help depend

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) $(TEST_OBJECTS)
	$(CC) $(OBJECTS) $(TEST_OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build successful: $@"

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled $<"

test: $(EXECUTABLE)
	@echo "Running tests..."
	./$(EXECUTABLE)

clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(EXECUTABLE) $(DEBUG_EXECUTABLE)
	@echo "Cleaned build artifacts"

# Debug target
debug:
	$(CC) $(CFLAGS) -g -DDEBUG $(SOURCES) $(TEST_SOURCES) -o $(DEBUG_EXECUTABLE) $(LDFLAGS)
	@echo "Debug build created: $(DEBUG_EXECUTABLE)"

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Build the test executable"
	@echo "  test    - Build and run tests"
	@echo "  clean   - Remove build artifacts"
	@echo "  debug   - Build debug version"
	@echo "  depend  - Print source dependencies"
	@echo "  help    - Show this help"

# Dependencies
depend: $(SOURCES) $(HEADERS) $(TEST_SOURCES)
	@echo "Dependencies:"
	@for file in $(SOURCES) $(TEST_SOURCES); do \
		echo "$$file:"; \
		$(CC) -MM $$file -I$(INCDIR) | sed 's/^/  /'; \
	done
