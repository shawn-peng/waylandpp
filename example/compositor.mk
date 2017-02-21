PROJDIR = ../

include $(PROJDIR)defs.mk
include $(PROJDIR)functions.mk
include $(PROJDIR)rules.mk

#TARGET = example

LIBS = wayland-server++ egl-renderer++ pixman-1 input weston-2 wayland-server unwind unwind-x86_64 dl

LDFLAGS += -Wl,-E

SRCS = \
	   compositor.cpp \
	   backend.cpp \
	   shared/config-parser.c \
	   shared/option-parser.c


#SRCS = \
#	   compositor.cpp \
#	   weston.c \
#	   text-backend.c \
#	   weston-screenshooter.c \
#	   shared/config-parser.c \
#	   shared/os-compatibility.c \
#	   shared/option-parser.c

#SRCS = \
#	   compositor.cpp \
#	   wayland-renderer.cpp



$(eval $(call make_executable,compositor,$(SRCS),$(LIBS)))

$(eval $(call print_vars,ALL_TARGETS))

all: $$(ALL_TARGETS)





