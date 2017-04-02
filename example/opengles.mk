PROJDIR = ../

include $(PROJDIR)defs.mk
include $(PROJDIR)functions.mk
include $(PROJDIR)rules.mk

#TARGET = example

LIBS = wayland-client++ wayland-egl++ wayland-cursor++ GLESv2 X11

SRCS = \
	   opengles.cpp


$(eval $(call make_executable,opengles,$(SRCS),$(LIBS)))

$(eval $(call print_vars,ALL_TARGETS))

all: $(ALL_TARGETS)





