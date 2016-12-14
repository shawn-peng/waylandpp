
include defs.mk
include functions.mk

export PROJDIR= $(shell pwd)/
export ALL_TARGETS

#TARGETS=client server

SOURCES:= $(shell find $(SRCDIR) -name '*.cpp')

SUBDIRS:= $(shell ls -d */)

.PHONY: all clean subdir info

all: subdir info $(ALL_TARGETS)#$(ALL_TAGETS)#$(addprefix $(BINDIR), $(BINARIES))

info: subdir
	$(eval $(call print_vars,ALL_TARGETS))

#-rm build/* -rf
clean: 
	-rm $(BINDIR)* -rf
	-rm $(LIBDIR)* -rf
	-rm $(OBJDIR)* -rf

#$(eval $(foreach d,$(SUBDIRS),make -C $(d);))

subdir:
	make -f src/Makefile all

#$(BINDIR)%: 

$(foreach target,$(BINARIES),$(eval $(call make_target,$(target))))

%:
	make -f $@.mk

$(OBJDIR)%.o: $(SRCDIR)%.cpp
	mkdir -p $(dir $@)
	$(CXX) -c $< $(CXXFLAGS) -o $@

VAR/TEST/A=This is a var test

test:
	$(info $(VAR/TEST/A))
	@echo

depend:
	 makedepend *.cpp

