include ../defs.mk

PROTOCOLS= wayland xdg-shell-unstable-v6
.PHONY: all $(PROTOCOLS)

all: $(PROTOCOLS)

$(PROTOCOLS): %: %.xml
	$(BINDIR)scanner $< $(INCDIR) $(SRCDIR)




