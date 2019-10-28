CC?=gcc
AR?=ar

LIBS=-lpthread

CPPFLAGS=-Iinclude/ -MMD -include include/def.h $(DEFINES)

ifeq ($(MALLOC_INFO), 1)
CPPFLAGS += -DMALLOC_INFO
endif

ifeq ($(ALLOCATOR_STATS), 1)
CPPFLAGS += -DALLOCATOR_STATS
endif

ifeq ($(DEBUG_MALOC), 1)
CPPFLAGS += -DDEBUG_MALLOC
endif

ifeq ($(ASSERT), 1)
CPPFLAGS += -DWITH_ASSERT
endif

define DIALECT
-std=gnu11
endef

define WFORMAT
-fdiagnostics-color=auto
endef

define WARNINGS
-Waggregate-return
-Wall
-Warray-bounds=2
-Wattribute-alias
-Wcast-align
-Wchar-subscripts
-Wdouble-promotion
-Wduplicated-branches
-Wduplicated-cond
-Wunreachable-code
-Wendif-labels
-Wextra
-Wfloat-equal
-Winit-self
-Winline
-Wcast-align
-Wmissing-include-dirs
-Wno-unused-parameter
-Wnonnull
-Wnonnull-compare
-Wnull-dereference
-Wshadow
-Wsign-compare
-Wswitch-enum
-Wstrict-overflow=5
-Wstringop-overflow=4
-Wno-stringop-truncation
-Wsuggest-attribute=cold
-Wsuggest-attribute=const
-Wsuggest-attribute=format
-Wsuggest-attribute=malloc
-Wsuggest-attribute=noreturn
-Wsuggest-attribute=pure
-Wuninitialized
-Wwrite-strings
endef

define DEBUG
-g
endef

define OPTIMIZATION
-O2
-fstrict-overflow
endef

define PROF
endef

define CFLAGS :=
$(DIALECT)
$(WFORMAT)
$(WARNINGS)
$(DEBUG)
$(OPTIMIZATION)
$(CODE)
$(PROF)
$(CFLAGS)
endef

ifeq ($(ADDR_SANITIZER), 1)
define CFLAGS :=
$(CFLAGS)
-ftrapv
-fsanitize-address-use-after-scope
-fsanitize=address
-fsanitize=alignment
-fsanitize=bounds
-fsanitize=bounds-strict
-fsanitize=integer-divide-by-zero
-fsanitize=leak
-fsanitize=nonnull-attribute
-fsanitize=null
-fsanitize=object-size
-fsanitize=pointer-compare
-fsanitize=pointer-overflow
-fsanitize=pointer-subtract
-fsanitize=returns-nonnull-attribute
-fsanitize=shift-base
-fsanitize=shift-exponent
-fsanitize=signed-integer-overflow
-fsanitize=undefined
-fsanitize=vla-bound
endef
CPPFLAGS += -DASAN
else ifeq ($(THREAD_SANITIZER), 1)
-fsanitize=thread
endif
CFLAGS:=$(call strip-newline, $(CFLAGS))

ifeq ($(RELEASE), 1)
CFLAGS = -O2 -Wall -Wextra
else ifeq ($(BLAME_GCC), 1)
CFLAGS = -O0 -g -Wall -Wextra -DDEBUG
endif

ifeq ($(MANIAC), 1)
CFLAGS += -Werror
endif
