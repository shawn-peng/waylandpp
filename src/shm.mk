include ../defs.mk
include ../functions.mk
include ../rules.mk

TARGET = libwayland-shm++.so

LIBS = wayland-server++

SRCS = \
	wayland-shm.cpp


$(eval $(call make_sharedlib,$(TARGET),$(SRCS),$(LIBS)))

$(eval $(call print_vars,ALL_TARGETS))


