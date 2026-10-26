#
# Makefile for geofence implementation
#
# Following Elecia White's principles:
# - Simple, clear build process
# - Separate compilation and linking
# - Easy testing integration
# - Cross-platform compatibility

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -I../../include
LDFLAGS = -lm

# Source files
SRCDIR = .
INCDIR = ../../include
TESTDIR = .

# Source files
SOURCES = $(SRCDIR)/geofence.c
HEADERS = $(INCDIR)/geofence.h
TEST_SOURCES = $(TESTDIR)/test_geofence.c

# Object files
OBJECTS = $(SOURCES:.c=.o)
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)

# Targets
TARGET = geofence_test
EXECUTABLE = $(TARGET)

.PHONY: all clean test

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
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(EXECUTABLE)
	@echo "Cleaned build artifacts"

# Debug target
debug:
	$(CC) $(CFLAGS) -g -DDEBUG $(SOURCES) $(TEST_SOURCES) -o $(EXECUTABLE)_debug $(LDFLAGS)
	@echo "Debug build created: $(EXECUTABLE)_debug"

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Build the test executable"
	@echo "  test    - Build and run tests"
	@echo "  clean   - Remove build artifacts"
	@echo "  debug   - Build debug version"
	@echo "  help    - Show this help"

# Dependencies
depend: $(SOURCES) $(HEADERS) $(TEST_SOURCES)
	@echo "Dependencies:"
	@for file in $(SOURCES) $(TEST_SOURCES); do \
		echo "$$file:"; \
		gcc -MM $$file -I$(INCDIR) | sed 's/^/  /'; \
	done