SHELL?=/bin/sh

destdir?=
prefix?=/usr/local
exec_prefix?=$(prefix)

# Executable for user
bindir?=$(exec_prefix)/bin

# Executable for program
libexecdir?=$(exec_prefix)/libexec

# Root directory for read-only architecture-independant data files
datarootdir?=$(prefix)/share

# Directory for installing read-only data files that pertain to a
# single machine
sysconfdir?=$(prefix)/etc

# Directory for installing architecture-independent data files which
# the programs modifiy while they run
localstatedir?=$(prefix)/var

# Directory for installing data files which the programs modify while
# they run, theat pertain to one specifig machine and persist longer
# than the execution of the program
runstatedir?=$(localstatedir)/run

# Directory for installing header files to be included by the user
# programs
includedir?=$(prefix)/include

# Directory for installing header files for use with compilers other
# than GCC
oldincludedir?=/usr/include

# Directory for installing locale-specifig message
localedir?=$(datarootdir)/locale
