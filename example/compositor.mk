PROJDIR = ../

include $(PROJDIR)defs.mk
include $(PROJDIR)functions.mk
include $(PROJDIR)rules.mk

#TARGET = example

LIBS = wayland-server++ pixman-1 input dl

LDFLAGS += -Wl,-E

SRCS = \
	   compositor.cpp \



$(eval $(call make_executable,compositor,$(SRCS),$(LIBS)))

$(eval $(call print_vars,ALL_TARGETS))

all: $$(ALL_TARGETS)





