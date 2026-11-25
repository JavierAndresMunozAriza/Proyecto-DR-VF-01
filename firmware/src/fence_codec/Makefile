#
# Makefile for the fence codec and its host test
#
# `make test` builds and runs the unit tests (including the shared vectors in
# test_vectors.txt) against the real geofence engine; `make sanitize` runs
# them under ASan and UBSan; `make clean` removes artifacts. Override the
# compiler with `make CC=clang`.

ifeq ($(origin CC),default)
CC = gcc
endif
CFLAGS = -Wall -Wextra -std=c99 -O2 -I../../include
LDFLAGS = -lm

TARGET = fence_codec_test
SOURCES = fence_codec.c test_fence_codec.c ../geofence/geofence.c
HEADERS = ../../include/fence_codec.h ../../include/geofence.h

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
