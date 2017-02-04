
PROJDIR:= $(abspath $(dir $(lastword $(MAKEFILE_LIST))))/

OPTS= -fpermissive -std=c++11
INCLUDES= -I/usr/local/include/ -I$(PROJDIR)include/

LIBPATHS= -L/usr/local/lib
COMMONLIBS= -lm -lwayland-egl -lEGL -lGL -lwld -lswc
#LIBS= -lm -lwayland-client -lwayland-server -lwayland-egl -lwayland-cursor -lEGL -lGL -lwld -lswc

MACROS=

CFLAGS= $(OPTS) $(INCLUDES)
CXXFLAGS= $(OPTS) $(INCLUDES)

BUILDDIR=$(PROJDIR)build/
BINDIR=$(BUILDDIR)bin/
LIBDIR=$(BUILDDIR)lib/
OBJDIR=$(BUILDDIR)obj/
INCDIR=$(PROJDIR)include/
SRCDIR=$(PROJDIR)src/

