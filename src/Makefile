
.PHONY: all

TARGETS = client server protocols

.PHONY: $(TARGETS)

all: $(TARGETS)

client server: protocols

$(TARGETS):
	make -f $@.mk



