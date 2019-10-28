PACKAGE=dirwatch
VERSION=0.0.1
PKG=$(PACKAGE)-$(VERSION)

-include local.mk
include mk/util.mk
include mk/std.mk
include mk/cc.mk
include mk/src.mk

ifndef VERBOSE
.SILENT:
endif

# Add to this list stuff to clean up
CLEAN=$(MKDIR) $(CLEAN_BISON)


# Build
all: $(MKDIR) $(GEN_BISON) $(TARGET)


$(MKDIR):
	$(foreach d,$@,$(call DO_MKDIR,$d))


$(TARGET): $(OBJ) | local.mk mk/src.mk mk/cc.mk
	$(QUIET_LINK)$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

-include $(OBJ:%.o=%.d)

%.c: %.y
	$(QUIET_YACC)bison -o $@ $<

$(BUILD)/%.o: %.c
	$(QUIET_CC)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

check: all
	cd ./test && ./run

# Distribute
define DISTRIBUTE
core/
mk/
Makefile
local.mk
endef

dist: $(PKG).tar.gz

distclean: clean
	$(call DO_RM, $(PKG))
	$(call DO_RM, $(PKG).tar.gz)

$(PKG):
	$(call DO_MKDIR, $@)
	$(foreach e,$(call strip-newline, $(DISTRIBUTE)), $(call DO_CP, $(PKG), $e))

$(PKG).tar.gz: $(PKG)
	$(call DO_TAR, $@,$<)



# Install
install_bin_dir=$(destdir)$(bindir)
install_data_dir=$(destdir)$(datarootdir)/$(TARGET_NAME)
install: all
	$(call DO_MKDIR, $(install_bin_dir))
	$(call DO_INST, $(TARGET), $(install_bin_dir)/$(TARGET_NAME))

uninstall:
	$(call DO_RM, $(destdir)$(bindir)/dirwatch)

# Util
clean:
	$(foreach i,$(CLEAN),$(call DO_RM, $i))


print-%:
	@echo $* = $($*)

help:
	@echo "PHONIES: "
	$(foreach p,$(PHONIES), echo -e "\t$p";)


define PHONIES
all
clean
dist
distclean
help
install
print
uninstall
endef
.PHONY: $(call strip-newline, $(PHONIES))
