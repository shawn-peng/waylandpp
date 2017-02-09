include ../defs.mk
include ../functions.mk
include ../rules.mk

TARGET = libwayland-cursor++.so

LIBS = wayland-client++ wayland-cursor

SRCS = \
	wayland-cursor.cpp


$(eval $(call make_sharedlib,$(TARGET),$(SRCS),$(LIBS)))

$(eval $(call print_vars,ALL_TARGETS))


