#
# Makefile for the fence manager and its host test
#
# `make test` builds and runs the unit tests against the real geofence engine
# and the real fence codec; `make sanitize` runs them under ASan and UBSan
# (which also finds leaks); `make clean` removes artifacts. Override the
# compiler with `make CC=clang`.

ifeq ($(origin CC),default)
CC = gcc
endif
CFLAGS = -Wall -Wextra -std=c99 -O2 -I../../include
LDFLAGS = -lm

TARGET = fence_manager_test
SOURCES = fence_manager.c test_fence_manager.c ../fence_codec/fence_codec.c \
	../geofence/geofence.c
HEADERS = ../../include/fence_manager.h ../../include/fence_store.h \
	../../include/fence_codec.h ../../include/geofence.h

.PHONY: test sanitize clean

test: $(TARGET)
	./$(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)

sanitize: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -g -fsanitize=address,undefined -fno-sanitize-recover=all \
		$(SOURCES) -o $(TARGET)_san $(LDFLAGS)
	./$(TARGET)_san

clean:
	rm -f $(TARGET) $(TARGET)_san
