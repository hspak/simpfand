CC = gcc
CFLAGS = -std=c99 -Wall -pedantic -Os

LDLIBS = -lm

SRC = simpfand.c
OBJ = $(SRC:.c=.o)

simpfand: simpfand.c
	$(CC) $(CFLAGS) $(LDLIBS) simpfand.c -o simpfand

strip: simpfand
	strip simpfand

install: simpfand
	install -D -m755 simpfand $(DESTDIR)$(PREFIX)/bin/simpfand

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/simpfand

clean:
	rm -f *.o simpfand
