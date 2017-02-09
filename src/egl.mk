
include ../defs.mk
include ../functions.mk
include ../rules.mk

TARGET = libwayland-egl++.so

LIBS = wayland-client++

SRCS = \
	wayland-egl.cpp


$(eval $(call make_sharedlib,$(TARGET),$(SRCS),$(LIBS)))

$(eval $(call print_vars,ALL_TARGETS))


