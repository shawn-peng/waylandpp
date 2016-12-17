include $(PROJDIR)defs.mk
include $(PROJDIR)functions.mk

TARGET = libwayland-client++.so

LIBS = wayland-client

SRCS = \
	wayland-client.cpp \
	wayland-client-protocol.cpp \
	wayland-util.cpp


$(eval $(call make_sharedlib,$(TARGET),$(SRCS),$(LIBS)))

$(eval $(call print_vars,ALL_TARGETS))


