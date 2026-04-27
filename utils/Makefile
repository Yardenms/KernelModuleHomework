# Variables
CC = gcc
CFLAGS = -Wall -O2

# The executable names
TARGETS = hello_world encryptor

all: $(TARGETS)

hello_world: hello_world.c
	$(CC) $(CFLAGS) hello_world.c -o hello_world

encryptor: encryptor.c
	$(CC) $(CFLAGS) encryptor.c -o encryptor

clean:
	rm -f $(TARGETS)
