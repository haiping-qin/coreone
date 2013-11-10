# Handle objects in subdirs
# ---------------------------------------------------------------------------
# o if we encounter foo/ in $(obj-y), replace it by foo/built-in.o
#   and add the directory to the list of dirs to descend into: $(subdir-y)

subdir-y	:= $(patsubst %/,%,$(filter %/, $(obj-y)))
obj-y		:= $(patsubst %/, %/built-in.o, $(obj-y))

# $(subdir-obj-y) is the list of objects in $(obj-y) which uses dir/ to
# tell kbuild to descend
subdir-obj-y	:= $(filter %/built-in.o, $(obj-y))

extra-y         := $(addprefix $(obj)/,$(extra-y))
obj-y		:= $(addprefix $(obj)/,$(obj-y))
subdir-obj-y	:= $(addprefix $(obj)/,$(subdir-obj-y))
subdir-y	:= $(addprefix $(obj)/,$(subdir-y))

__c_flags	= $(CPPFLAGS) $(CFLAGS) $(ccflags-y) $(CFLAGS_$(basetarget).o)
__a_flags	= $(CPPFLAGS) $(AFLAGS) $(asflags-y) $(AFLAGS_$(basetarget).o)
__cpp_flags     = $(CPPFLAGS) $(cppflags-y) $(CPPFLAGS_$(@F))

c_flags		= -Wp,-MD,$(depfile) $(NOSTDINC_FLAGS) $(INCLUDES) $(__c_flags)
a_flags		= -Wp,-MD,$(depfile) $(NOSTDINC_FLAGS) $(INCLUDES) $(__a_flags)
cpp_flags	= -Wp,-MD,$(depfile) $(NOSTDINC_FLAGS) $(INCLUDES) $(__cpp_flags)
ld_flags	= $(LDFLAGS) $(ldflags-y)

# Commands useful for building a boot image
# ===========================================================================
# 
#	Use as following:
#
#	target: source(s) FORCE
#		$(if_changed,ld/objcopy/gzip)
#
#	and add target to extra-y so that we know we have to
#	read in the saved command line

# Linking
# ---------------------------------------------------------------------------

quiet_cmd_ld = LD      $@
cmd_ld = $(LD) $(LDFLAGS) $(ldflags-y) $(LDFLAGS_$(@F)) \
	       $(filter-out FORCE,$^) -o $@ 

# Objcopy
# ---------------------------------------------------------------------------

quiet_cmd_objcopy = OBJCOPY $@
cmd_objcopy = $(OBJCOPY) $(OBJCOPYFLAGS) $(OBJCOPYFLAGS_$(@F)) $< $@

# Gzip
# ---------------------------------------------------------------------------

quiet_cmd_gzip = GZIP    $@
cmd_gzip = (cat $(filter-out FORCE,$^) | gzip -n -f -9 > $@) || \
	(rm -f $@ ; false)

quote:="

