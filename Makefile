CC = gcc
CFLAGS = -std=c99 -Wall -pedantic -Os

LDLIBS = -lm

install: simpfand
	install -D -m755 simpfand $(DESTDIR)$(PREFIX)/bin/simpfand

strip: simpfand
	strip simpfand

simpfand: simpfand.c
	$(CC) $(CFLAGS) $(LDLIBS) simpfand.c -o simpfand

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/simpfand

clean:
	rm -f *.o simpfand
