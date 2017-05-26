
include ../defs.mk
include ../functions.mk
include ../rules.mk

TARGET = libxdg_shell_unstable_v6-server++.so

LIBS = wayland-server++

SRCS = \
	xdg_shell_unstable_v6-server-protocol.cpp


$(eval $(call make_sharedlib,$(TARGET),$(SRCS),$(LIBS)))

$(eval $(call print_vars,ALL_TARGETS))


