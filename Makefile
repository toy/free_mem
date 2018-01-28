CFLAGS = -Wall -Wextra -Werror

DESTDIR =
prefix = /usr/local
bindir = $(prefix)/bin
INSTALL = install
INSTALL_PROGRAM = $(INSTALL) -m 755

build: free_mem

clean:
	$(RM) free_mem

install: build
	$(INSTALL_PROGRAM) free_mem $(DESTDIR)$(bindir)/free_mem

uninstall:
	$(RM) $(DESTDIR)$(bindir)/free_mem

.PHONY: build clean install uninstall
