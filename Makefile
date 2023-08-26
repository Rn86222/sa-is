CC = gcc
CFLAGS = -Wall -O3
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

TARGET = sais

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) $(TARGET) *.o  *~
