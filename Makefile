# Define the compiler and flags
CC = gcc
CFLAGS = -Wall -g -pthread `pkg-config --cflags glib-2.0`
LDFLAGS = `pkg-config --libs glib-2.0`

# Define the target programs
SERVER = server
CLIENT = client

# Source files
SERVER_SRC = server.c
CLIENT_SRC = client.c

# Object files
SERVER_OBJ = $(SERVER_SRC:.c=.o)
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)

# Default target (build both the server and client)
all: $(SERVER) $(CLIENT)

# Rule to build the server
$(SERVER): $(SERVER_OBJ)
	$(CC) -o $(SERVER) $(SERVER_OBJ) $(LDFLAGS)

# Rule to build the client
$(CLIENT): $(CLIENT_OBJ)
	$(CC) -o $(CLIENT) $(CLIENT_OBJ) $(LDFLAGS)

# Clean object files and executables
clean:
	rm -f $(SERVER_OBJ) $(CLIENT_OBJ) $(SERVER) $(CLIENT)

# Rule to build the object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Install dependencies (if any additional ones are needed)
install:
	sudo apt-get install libglib2.0-dev


