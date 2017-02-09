PROJDIR = ../

include $(PROJDIR)defs.mk
include $(PROJDIR)functions.mk
include $(PROJDIR)rules.mk

TARGET = example

LIBS = wayland-client++ wayland-egl++ wayland-cursor++

SRCS = \
	   opengles.cpp


$(eval $(call make_executable,$(TARGET),$(SRCS),$(LIBS)))

$(eval $(call print_vars,ALL_TARGETS))





