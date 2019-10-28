# Uncomment to install into your home directory
#destdir=$(HOME)

# Where to build the object files.  To build the binary faster, select
# a tmpfs mount point.  /tmp is usualy a good candidate.
BUILD=/tmp/$(PKG)

# Set to 1 if you compile for release
RELEASE=1


# Variables bellow are for developper
# Recommended C flags during developpement
CFLAGS = -O0 -ggdb3 -DDEBUG

# Set to 1 if you want to use AddressSanitizer
ADDR_SANITIZER=0

# Set to 1 if you want to use ThreadSanitizer.
# Incompatible with ADDR_SANITIZER
THREAD_SANITIZER=0

# Set to 1 if you want to print malloc informations
MALLOC_INFO=0

ASSERT=1

# Set to 1 if you want to print allocators' informations
ALLOCATOR_STATS=1

# Set to 1 if all allocations should use malloc
DEBUG_MALOC=1

# Set to 1 if you think GCC is to blame
BLAME_GCC=0

# Set to 1 if you want all warnings to be errors
MANIAC=0
