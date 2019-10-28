TARGET_NAME=dirwatch
TARGET_DIR=bin
TARGET=$(TARGET_DIR)/$(TARGET_NAME)
MKDIR=$(TARGET_DIR) $(BUILD)

define DO_SRC
	$(filter-out $(GEN_BISON),$(wildcard $1/*.c))
endef

define GEN_MKDIR
	$(BUILD)/$1
endef

# Where to find source files
define SRC_DIR
endef

# YACC
define BISON
endef
BISON:=$(call strip-newline,$(BISON))
GEN_BISON=$(BISON:%.y=%.c)
CLEAN_BISON=$(GEN_BISON) $(GEN_BISON:%.c=%.h)

# Directories to create
MKDIR:=$(MKDIR) $(foreach s,$(SRC_DIR),$(call GEN_MKDIR,$s))

# Generate list of sources and objects
SRC=$(GEN_BISON) $(foreach s,$(SRC_DIR),$(call DO_SRC,$s))
OBJ=$(SRC:%.c=$(BUILD)/%.o)
