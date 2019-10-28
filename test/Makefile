-include ../local.mk
include ../mk/util.mk
include ../mk/std.mk
include ../mk/cc.mk

CPPFLAGS:=$(filter-out -I% -include include% -DMALLOC_INFO -DALLOCATOR_STATS, $(CPPFLAGS))
CPPFLAGS += -I../include -include ../include/def.h
SRC=$(wildcard *.c)
OBJ=$(SRC:%.c=%.o)
OUT=$(SRC:%.c=%.out)

ifndef VERBOSE
.SILENT:
endif

all: $(OUT)

%.out: %.o
	$(QUIET_LINK)$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(QUIET_CC)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(foreach i,$(wildcard ./*.o ./*.d ./*.out),$(call DO_RM, $i))

.PHONY: clean
