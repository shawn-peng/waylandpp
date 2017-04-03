
include ../defs.mk

.PHONY: all

all: $(BINDIR)scanner

$(BINDIR)scanner: scanner.cpp pugixml.cpp
	@mkdir -p $(BINDIR)
	$(CXX) -o $@ $^ -I./ -I/usr/local/include




