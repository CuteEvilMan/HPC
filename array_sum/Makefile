# Compiler settings
CC = gcc
CFLAGS = -O3 -mavx2 -Wall -Wextra -pthread  -Werror
LDFLAGS = 
LDLIBS = -lpthread
TARGET = parallel_sum
SRC = pthread.c

# Build rules
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean

