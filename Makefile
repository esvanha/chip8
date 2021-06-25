CC = gcc
CFLAGS = -Wall -O3 -s -std=c99
LDFLAGS = -lSDL2

TARGET_EXEC = chip8
BUILD_DIR = build

SRCS := $(shell find . -name '*.c')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/%.c.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET_EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET_EXEC)
