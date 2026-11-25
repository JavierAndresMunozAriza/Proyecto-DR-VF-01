#
# Makefile for the geofence engine and its host test
#
# Following Elecia White's principles:
# - Simple, clear build process
# - Separate compilation and linking
# - Easy testing integration
#
# `make test` builds and runs the unit tests (test_geofence.c, one binary).
# `make stress`, `make sanitize` and `make cross-check` add the randomized
# stress test, the sanitizer runs and the compile check for both boards; see
# `make help`. Override the compiler with `make CC=clang`.

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

# The other pure-C modules: `make cross-check` compiles them too, for the host and both targets.
PURE_MODULES = ../alert/alert_color.c ../fence/fence_square.c ../app/app_fsm.c \
	../demo/demo_route.c ../bench/bench_polygon.c ../fence_codec/fence_codec.c
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

.PHONY: all test stress sanitize cross-check clean debug help depend

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

# Compile (no link) the engine for both target boards with the Zephyr SDK
# compilers and strict warnings treated as errors: Cortex-M33 (nRF9151,
# Thingy:91 X) and Xtensa LX7 (ESP32-S3). Prints the object size.
ZEPHYR_SDK ?= $(HOME)/zephyr-sdk-1.0.1
SDK_GNU = $(ZEPHYR_SDK)/gnu
CM33_PREFIX = $(SDK_GNU)/arm-zephyr-eabi/bin/arm-zephyr-eabi-
ESP32S3_PREFIX = $(SDK_GNU)/xtensa-espressif_esp32s3_zephyr-elf/bin/xtensa-espressif_esp32s3_zephyr-elf-
CM33_FLAGS = -mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16
STRICT_FLAGS = -std=c99 -O2 -Wall -Wextra -Wpedantic -Werror -Wshadow \
	-Wconversion -Wdouble-promotion -Wcast-qual -Wstrict-prototypes \
	-Wmissing-prototypes -Wundef -I$(INCDIR)
CROSS_OBJECTS = geofence_cm33.o geofence_esp32s3.o

cross-check: $(SOURCES) $(HEADERS)
	@test -x $(CM33_PREFIX)gcc || { echo "missing $(CM33_PREFIX)gcc (set ZEPHYR_SDK)"; exit 1; }
	@test -x $(ESP32S3_PREFIX)gcc || { echo "missing $(ESP32S3_PREFIX)gcc (set ZEPHYR_SDK)"; exit 1; }
	$(CC) $(STRICT_FLAGS) -c $(SOURCES) -o /dev/null
	$(CM33_PREFIX)gcc $(CM33_FLAGS) $(STRICT_FLAGS) -c $(SOURCES) -o geofence_cm33.o
	$(ESP32S3_PREFIX)gcc $(STRICT_FLAGS) -c $(SOURCES) -o geofence_esp32s3.o
	$(CM33_PREFIX)size geofence_cm33.o
	$(ESP32S3_PREFIX)size geofence_esp32s3.o
	@for m in $(PURE_MODULES); do \
		echo "cross-check $$m"; \
		$(CC) $(STRICT_FLAGS) -c $$m -o /dev/null || exit 1; \
		$(CM33_PREFIX)gcc $(CM33_FLAGS) $(STRICT_FLAGS) -c $$m -o /dev/null || exit 1; \
		$(ESP32S3_PREFIX)gcc $(STRICT_FLAGS) -c $$m -o /dev/null || exit 1; \
	done

clean:
	rm -f $(CROSS_OBJECTS)
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
	@echo "  all         - Build the unit test executable"
	@echo "  test        - Build and run the unit tests"
	@echo "  stress      - Run the randomized stress test (SEED=n changes the seed)"
	@echo "  sanitize    - Run the tests and the stress test under ASan and UBSan"
	@echo "  cross-check - Compile the engine for Cortex-M33 and ESP32-S3, strict warnings"
	@echo "  clean       - Remove build artifacts"
	@echo "  debug       - Build the unit tests with debug flags"
	@echo "  depend      - Print source dependencies"
	@echo "  help        - Show this help"

# Dependencies
depend: $(SOURCES) $(HEADERS) $(TEST_SOURCES)
	@echo "Dependencies:"
	@for file in $(SOURCES) $(TEST_SOURCES); do \
		echo "$$file:"; \
		$(CC) -MM $$file -I$(INCDIR) | sed 's/^/  /'; \
	done
