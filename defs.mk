
PROJDIR:= $(abspath $(dir $(lastword $(MAKEFILE_LIST))))/

BUILDDIR=$(PROJDIR)build/
BINDIR=$(BUILDDIR)bin/
LIBDIR=$(BUILDDIR)lib/
OBJDIR=$(BUILDDIR)obj/
INCDIR=$(PROJDIR)include/
SRCDIR=$(PROJDIR)src/

OPTS= -fpermissive -std=c++11 -fPIC -g

INCLUDES= -I/usr/local/include/ -I$(INCDIR)

LIBPATHS= -L/usr/local/lib -L$(LIBDIR)

COMMONLIBS= -lm -lwayland-egl -lEGL -lGL -lwld -lswc
#LIBS= -lm -lwayland-client -lwayland-server -lwayland-egl -lwayland-cursor -lEGL -lGL -lwld -lswc

MACROS=

CFLAGS= $(OPTS) $(INCLUDES)
CXXFLAGS= $(OPTS) $(INCLUDES)
LDFLAGS= $(OPTS) $(LIBPATHS) $(COMMONLIBS)

