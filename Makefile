CC = gcc
CFLAGS = -std=c99 -Wall -pedantic -Os

LDLIBS = -lm

SRC = simpfand.c parse.c
OBJ = $(SRC:.c=.o)
	
simpfand: $(OBJ)

strip: simpfand
	strip --strip-all simpfand

install: simpfand
	install -D -m755 simpfand $(DESTDIR)$(PREFIX)/bin/simpfand
	install -D -m644 simpfand.conf $(DESTDIR)/etc/conf.d/simpfand

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/simpfand
	rm -f $(DESTDIR)$(PREFIX)/etc/conf.d/simpfand

clean:
	rm -f *.o simpfand

.PHONY: install uninstall clean
