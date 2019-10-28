define newline


endef

empty=
space=$(empty) $(empty)

define strip-newline
$(subst $(newline),$(space),$1)
endef
# ^ Is a hack.  Allows us to define a list of definitions with
# define/endef and strip newline from it.  The resulting definition is
# now a single list of item, good for command line.  No more
# backslashes in definitions.


RM=rm -f
RMDIR=rm -rf
INSTALL=install

# To use in rules only
QUIET_CC      = @echo    '     CC       ' $@;
QUIET_GEN     = @echo    '     GEN      ' $@;
QUIET_LINK    = @echo    '     LINK     ' $@;
QUIET_YACC    = @echo    '     YACC     ' $@;


# To use in macros only
QUIET_MKDIR   = echo     '     MKDIR    ' $1;
QUIET_RM      = echo     '     RM       ' $1;
QUIET_RMDIR   = echo     '     RMDIR    ' $1;
QUIET_TAR     = echo     '     TAR      ' $1 ' <- ' $2;
QUIET_CP      = echo     '     CP       ' $1 ' <- ' $2;
QUIET_INST    = echo     '     INSTALL  ' $2;

define DO_MKDIR
	if [ ! -d $1 ]; then $(QUIET_MKDIR) mkdir -p $1; fi;
endef

define DO_TAR
	$(QUIET_TAR) tar -zcf $1 $2;
endef

define DO_RM
	if [ -f $1 ]; then $(QUIET_RM)$(RM) $1; elif [ -d $1 ]; then $(QUIET_RMDIR)$(RMDIR) $1; fi;
endef

define DO_INST
	$(QUIET_INST)cp --recursive $1 $2
endef
