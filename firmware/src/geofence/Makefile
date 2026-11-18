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
STRESS_EXECUTABLE = $(TARGET)_stress
SANITIZE_EXECUTABLE = $(TARGET)_sanitize
SANITIZE_STRESS_EXECUTABLE = $(TARGET)_sanitize_stress
STRESS_SOURCES = $(TESTDIR)/test_geofence_stress.c
SANITIZE_FLAGS = -g -O1 -fno-omit-frame-pointer \
	-fsanitize=address,undefined,float-divide-by-zero,float-cast-overflow \
	-fno-sanitize-recover=all

.PHONY: all test stress sanitize clean debug help depend

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

# Randomized stress test: hundreds of thousands of cases, fixed seed.
# Pass another seed with `make stress SEED=123`.
stress: $(STRESS_EXECUTABLE)
	./$(STRESS_EXECUTABLE) $(SEED)

$(STRESS_EXECUTABLE): $(SOURCES) $(STRESS_SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) $(STRESS_SOURCES) -o $@ $(LDFLAGS)

# The unit tests and the stress test under AddressSanitizer and
# UndefinedBehaviorSanitizer (also reports memory leaks).
sanitize: $(SOURCES) $(TEST_SOURCES) $(STRESS_SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SANITIZE_FLAGS) $(SOURCES) $(TEST_SOURCES) -o $(SANITIZE_EXECUTABLE) $(LDFLAGS)
	$(CC) $(CFLAGS) $(SANITIZE_FLAGS) $(SOURCES) $(STRESS_SOURCES) -o $(SANITIZE_STRESS_EXECUTABLE) $(LDFLAGS)
	./$(SANITIZE_EXECUTABLE)
	./$(SANITIZE_STRESS_EXECUTABLE) $(SEED)

clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(EXECUTABLE) $(DEBUG_EXECUTABLE) \
		$(STRESS_EXECUTABLE) $(SANITIZE_EXECUTABLE) $(SANITIZE_STRESS_EXECUTABLE)
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
	@echo "  stress  - Run the randomized stress test (SEED=n to change the seed)"
	@echo "  sanitize - Run the tests and the stress test under ASan and UBSan"
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
