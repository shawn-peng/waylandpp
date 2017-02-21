
include ../defs.mk
include ../functions.mk
include ../rules.mk

TARGET = libwayland-server++.so

LIBS = wayland-server

SRCS = \
	wayland-server.cpp \
	wayland-server-protocol.cpp \
	wayland-protocol.cpp \
	wayland-util.cpp

$(eval $(call make_sharedlib,$(TARGET),$(SRCS),$(LIBS)))

all: $(ALL_TARGETS)


