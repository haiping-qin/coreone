VERSION = 2013
PATCHLEVEL = 10
SUBLEVEL =
EXTRAVERSION =
Name =

KERNELVERSION = $(VERSION)$(if $(PATCHLEVEL),.$(PATCHLEVEL)$(if $(SUBLEVEL),.$(SUBLEVEL)))$(EXTRAVERSION)

MAKEFLAGS += -rR --no-print-directory

# To put more focus on warnings, be less verbose as default
# Use 'make V=1' to see the full commands

ifeq ("$(origin V)", "command line")
  BUILD_VERBOSE := $(V)
endif
ifeq ($(BUILD_VERBOSE),1)
  quiet =
  Q =
else
  quiet=quiet_
  Q = @
endif

# If the user is running make -s (silent mode), suppress echoing of
# commands

ifneq ($(filter s% -s%,$(MAKEFLAGS)),)
  quiet=silent_
endif

export quiet Q

HOSTARCH := $(shell uname -m | sed -e s/i.86/i386/ -e s/sun4u/sparc64/ \
				  -e s/arm.*/arm/ -e s/sa110/arm/ \
				  -e s/s390x/s390/ -e s/parisc64/parisc/ \
				  -e s/ppc.*/powerpc/ -e s/mips.*/mips/ \
				  -e s/sh[234].*/sh/ -e s/aarch64.*/arm64/ )

HOSTOS := $(shell uname -s | tr '[:upper:]' '[:lower:]' | \
	    sed -e 's/\(cygwin\).*/windows/')

ARCH		?= $(HOSTARCH)
CROSS_COMPILE	?= $(CONFIG_CROSS_COMPILE:"%"=%)

SRCARCH 	:= $(ARCH)

# Additional ARCH settings for x86
ifeq ($(ARCH),i386)
        SRCARCH := x86
endif
ifeq ($(ARCH),x86_64)
        SRCARCH := x86
endif

# Additional ARCH settings for sparc
ifeq ($(ARCH),sparc32)
       SRCARCH := sparc
endif
ifeq ($(ARCH),sparc64)
       SRCARCH := sparc
endif

# Additional ARCH settings for sh
ifeq ($(ARCH),sh64)
       SRCARCH := sh
endif

# Additional ARCH settings for tile
ifeq ($(ARCH),tilepro)
       SRCARCH := tile
endif
ifeq ($(ARCH),tilegx)
       SRCARCH := tile
endif

export	HOSTARCH HOSTOS SRCARCH

# Default target
PHONY := all
all:

srctree		:= $(CURDIR)
objtree		:= $(CURDIR)

# Look for make include files relative to root of kernel src
MAKEFLAGS += --include-dir=$(srctree)

# We need some generic definitions (do not try to remake the file).
tools/make/include.mk: ;
include tools/make/include.mk

# Make variables (CC, etc...)

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
AWK		= awk
SHELL		= sh

INCLUDES	:= -Iinclude -Iarch/$(ARCH)/include \
		   -include include/generated/config.h
CFLAGS		:= -O2 -g -fno-short-enums -fno-builtin -finline \
		   -W -Wall -Wno-multichar -Wno-unused-parameter \
		   -Wno-unused-function -ffunction-sections -fdata-sections \
		   -D_LINUX_ -nostdinc
AFLAGS		:= -g -DASSEMBLY
NOSTDINC_FLAGS	:= -nostdinc -isystem $(shell $(CC) -print-file-name=include)

export CC CPP AS LD AR NM OBJCOPY OBJDUMP STRIP AWK SHELL
export INCLUDES CFLAGS CPPFLAGS AFLAGS LDFLAGS NOSTDINC_FLAGS

export BUILD_LDS	:= arch/$(SRCARCH)/kernel/one.lds
export LDFLAGS_one

export DOT_CONFIG	:= .config
export SRC_CONFIG	:= include/generated/config.h
export BUILD_CONFIG	:= include/generated/autoconf.mk

ifneq ($(filter %_config,$(MAKECMDGOALS)),)

%_config: tools/make/conf.sh FORCE
	$(Q)mkdir -p include/generated
	$(Q)$(SHELL) $< configs/$@

else

-include $(BUILD_CONFIG)

all: one

core-y		:= kernel/
#drivers-y	:= drivers/
libs-y		:= lib/
#app-y		:= app/

include arch/$(SRCARCH)/Makefile

one-dirs	:= $(patsubst %/,%, $(filter %/, $(core-y) $(drivers-y) $(libs-y) $(app-y)))
one-objs	:= $(patsubst %,%/built-in.o, $(one-dirs))

one-deps := $(BUILD_LDS) $(one-objs)

# Final link of one
quiet_cmd_link-one = LINK    $@
      cmd_link-one = $(LD) -T $(BUILD_LDS) $(one-objs) $(LDFLAGS) $(LDFLAGS_one) -Map $@.map -o $@

one: $(one-deps) FORCE
	$(call if_changed,link-one)

# The actual objects are generated when descending, 
# make sure no implicit rule kicks in
$(sort $(one-deps)): $(one-dirs) ;

PHONY += $(one-dirs)
$(one-dirs): prepare
	$(Q)$(MAKE) $(build)=$@

PHONEY += prepare
prepare: prepare_conf
	$(Q)$(MAKE) $(build)=.

PHONEY += prepare_conf
prepare_conf: tools/make/conf.sh
	$(Q)test -e $(SRC_CONFIG) -a -e $(BUILD_CONFIG) || (	\
	  echo >&2 No configuration specified; false)
	$(Q)cmp -s $(BUILD_CONFIG) $(DOT_CONFIG) || (		\
	  $(SHELL) $< -u $(DOT_CONFIG))

clean:
	@find . \( -name '*.[os]' -o -name '.*.cmd' -o -name '.*.d' -o -name '.*.tmp' \
		-o -name lib.a \) -type f -print | xargs rm -f
	@rm -f one one.map $(BUILD_LDS)

distclean:
	@find . -name '.*.swp' -delete
	@rm -rf cscope* tags include/generated/* .config

endif

# read all saved dependency lines
#
targets := $(wildcard $(sort $(targets)))
cmd_files := $(wildcard .*.cmd $(foreach f,$(targets),$(dir $(f)).$(notdir $(f)).cmd))

ifneq ($(cmd_files),)
  $(cmd_files): ;        # Do not try to update included dependency files
  include $(cmd_files)
endif

PHONY += FORCE
FORCE:

.PHONY: $(PHONY)
