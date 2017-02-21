
PROJDIR:= $(abspath $(dir $(lastword $(MAKEFILE_LIST))))/

BUILDDIR=$(PROJDIR)build/
BINDIR=$(BUILDDIR)bin/
LIBDIR=$(BUILDDIR)lib/
OBJDIR=$(BUILDDIR)obj/
INCDIR=$(PROJDIR)include/
SRCDIR=$(PROJDIR)src/

OPTS= -fpermissive -std=c++11 -fPIC -g

WESTON_PREFIX=/home/shawn/singularity-workspace/weston/install/

INCLUDES= -I$(WESTON_PREFIX)include/ -I$(WESTON_PREFIX)include/libweston-2/ -I/usr/local/include/ -I/usr/include/pixman-1/ -I/usr/include/libdrm/ -I$(INCDIR)

LIBPATHS= -L$(WESTON_PREFIX)lib/ -L/usr/local/lib -L$(LIBDIR)

COMMONLIBS= -lm -lwayland-egl -lEGL -lGL -lwld -lswc
#LIBS= -lm -lwayland-client -lwayland-server -lwayland-egl -lwayland-cursor -lEGL -lGL -lwld -lswc

MACROS= -DLIBEXECDIR='"$(WESTON_PREFIX)libexec"' \
		-DMODULEDIR='"$(WESTON_PREFIX)lib/weston"'

CFLAGS= $(OPTS) $(INCLUDES) $(MACROS)
CXXFLAGS= $(OPTS) $(INCLUDES) $(MACROS)
LDFLAGS= $(OPTS) $(LIBPATHS) $(COMMONLIBS)

