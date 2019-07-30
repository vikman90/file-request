# July 30, 2019

TARGET := bin/server bin/client
CFLAGS := -pipe -O2 -g -Wall -Wextra -std=c99
LFLAGS := -O2 -g

SOURCES := $(wildcard src/*.c)
OBJECTS := $(SOURCES:.c=.o)
HEADERS := $(wildcard src/*.h)

.PHONY: all clean

all: bin $(TARGET)

bin/client: src/client.o src/network.o src/shared.o
	$(CC) $(LFLAGS) -o $@ $^

bin/server: src/network.o src/server.o src/shared.o
	$(CC) $(LFLAGS) -o $@ $^

$(OBJECTS): $(HEADERS)

bin:
	mkdir $@

clean:
	$(RM) $(TARGET) $(OBJECTS)