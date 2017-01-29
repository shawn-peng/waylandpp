
include ../defs.mk

.PHONY: all

all: $(BINDIR)scanner

$(BINDIR)scanner: scanner.cpp pugixml.cpp
	$(CXX) -o $@ $^ -I./ -I/usr/local/include




