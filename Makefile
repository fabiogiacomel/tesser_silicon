CC = gcc
CFLAGS = -Wall -I./src
SRC_DIR = src
OBJ = $(wildcard $(SRC_DIR)/*.c)
TARGET = tesser_tower

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) $(SRC_DIR)/*.o

.PHONY: all clean
