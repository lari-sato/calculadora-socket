CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
SRC_DIR = src
INC_DIR = include

all: server client

server: $(SRC_DIR)/server.c $(INC_DIR)/proto.h
	$(CC) $(CFLAGS) -o $@ $(SRC_DIR)/server.c

client: $(SRC_DIR)/client.c $(INC_DIR)/proto.h
	$(CC) $(CFLAGS) -o $@ $(SRC_DIR)/client.c

clean:
	rm -f server client