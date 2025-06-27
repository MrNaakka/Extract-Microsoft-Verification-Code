
CC = gcc
CFLAGS = -g -O0 -Wall -Wextra -pedantic
EXECUTABLE = main
CFILES = main.c
LFLAGS = -lsqlite3

all: $(EXECUTABLE)

$(EXECUTABLE): $(CFILES)
	$(CC) $(CFLAGS) $(CFILES) $(LFLAGS) -o $(EXECUTABLE)

clean: 
	rm -f $(EXECUTABLE)

