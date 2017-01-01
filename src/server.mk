include $(PROJDIR)defs.mk
include $(PROJDIR)functions.mk

TARGET = server

LIBS = wayland-server

SRCS = \
	wayland-server.cpp \
	wayland-server-protocol.cpp \
	wayland-util.cpp

$(info $(call make_sharedlib,$(TARGET),$(SRCS),$(LIBS)))

all: $(ALL_TARGETS)


