# Makefile per compilare il progetto

# Compilatore
CC = gcc

# Nome eseguibile
TARGET1 = bin/TriServer
TARGET2 = bin/TriClient

# file sorgente
SRCS = src/TriServer.c src/fun.c
SRCS2 = src/TriClient.c src/fun.c

# Compilazione

all: $(TARGET1) TriClient

$(TARGET1): $(SRCS)
	$(CC) -o $(TARGET1) $(SRCS)

TriClient: $(SRCS2)
	$(CC) -o $(TARGET2) $(SRCS2)

# Pulizia
clean:
	rm -f $(TARGET1) $(TARGET2)
