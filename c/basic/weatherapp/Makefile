CC = gcc
CFLAGS = -O2 -Wall
LIBS = -lcurl -lcjson

all: weather

weather: weatherapp.c
	$(CC) $(CFLAGS) -o $@ weatherapp.c $(LIBS)

clean:
	rm -f weather

.PHONY: all clean
