PROJDIR = ../

include $(PROJDIR)defs.mk
include $(PROJDIR)functions.mk
include $(PROJDIR)rules.mk

#TARGET = example

LIBS = wayland-server++ pixman-1 input dl EGL wayland-client++ wayland-egl++ wayland-cursor++ wayland-shm++ GLESv2 #wayland-server

LDFLAGS += -Wl,-E

SRCS = \
	   compositor.cpp \
	   wrapper.cpp \




$(eval $(call make_executable,compositor,$(SRCS),$(LIBS)))

$(eval $(call print_vars,ALL_TARGETS))

all: $$(ALL_TARGETS)





