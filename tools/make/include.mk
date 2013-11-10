####
# kbuild: Generic definitions

# Convenient variables
comma   := ,
squote  := '
empty   :=
space   := $(empty) $(empty)

###
# Name of target with a '.' as filename prefix. foo/bar.o => foo/.bar.o
dot-target = $(dir $@).$(notdir $@)

###
# The temporary file to save gcc -MD generated dependencies must not
# contain a comma
depfile = $(subst $(comma),_,$(dot-target).d)

###
# filename of target with directory and extension stripped
basetarget = $(basename $(notdir $@))

###
# filename of first prerequisite with directory and extension stripped
baseprereq = $(basename $(notdir $<))

###
# Escape single quote for use in echo statements
escsq = $(subst $(squote),'\$(squote)',$1)

###
# Easy method for doing a status message
       kecho := :
 quiet_kecho := echo
silent_kecho := :
kecho := $($(quiet)kecho)

###
# filechk is used to check if the content of a generated file is updated.
# Sample usage:
# define filechk_sample
#	echo $KERNELRELEASE
# endef
# version.h : Makefile
#	$(call filechk,sample)
# The rule defined shall write to stdout the content of the new file.
# The existing file will be compared with the new one.
# - If no file exist it is created
# - If the content differ the new file is used
# - If they are equal no change, and no timestamp update
# - stdin is piped in from the first prerequisite ($<) so one has
#   to specify a valid file as first prerequisite (often the kbuild file)
define filechk
	$(Q)set -e;				\
	$(kecho) '  CHK     $@';		\
	mkdir -p $(dir $@);			\
	$(filechk_$(1)) < $< > $@.tmp;		\
	if [ -r $@ ] && cmp -s $@ $@.tmp; then	\
		rm -f $@.tmp;			\
	else					\
		$(kecho) '  UPD     $@';	\
		mv -f $@.tmp $@;		\
	fi
endef

######
# gcc support functions
# See documentation in Documentation/kbuild/makefiles.txt

# cc-cross-prefix
# Usage: CROSS_COMPILE := $(call cc-cross-prefix, m68k-linux-gnu- m68k-linux-)
# Return first prefix where a prefix$(CC) is found in PATH.
# If no $(CC) found in PATH with listed prefixes return nothing
cc-cross-prefix =  \
	$(word 1, $(foreach c,$(1),                                   \
		$(shell set -e;                                       \
		if (which $(strip $(c))$(CC)) > /dev/null 2>&1 ; then \
			echo $(c);                                    \
		fi)))

# output directory for tests below
TMPOUT := $(if $(KBUILD_EXTMOD),$(firstword $(KBUILD_EXTMOD))/)

# try-run
# Usage: option = $(call try-run, $(CC)...-o "$$TMP",option-ok,otherwise)
# Exit code chooses option. "$$TMP" is can be used as temporary file and
# is automatically cleaned up.
try-run = $(shell set -e;		\
	TMP="$(TMPOUT).$$$$.tmp";	\
	TMPO="$(TMPOUT).$$$$.o";	\
	if ($(1)) >/dev/null 2>&1;	\
	then echo "$(2)";		\
	else echo "$(3)";		\
	fi;				\
	rm -f "$$TMP" "$$TMPO")

# as-option
# Usage: cflags-y += $(call as-option,-Wa$(comma)-isa=foo,)

as-option = $(call try-run,\
	$(CC) $(KBUILD_CFLAGS) $(1) -c -x assembler /dev/null -o "$$TMP",$(1),$(2))

# as-instr
# Usage: cflags-y += $(call as-instr,instr,option1,option2)

as-instr = $(call try-run,\
	printf "%b\n" "$(1)" | $(CC) $(KBUILD_AFLAGS) -c -x assembler -o "$$TMP" -,$(2),$(3))

# cc-option
# Usage: cflags-y += $(call cc-option,-march=winchip-c6,-march=i586)

cc-option = $(call try-run,\
	$(CC) $(KBUILD_CPPFLAGS) $(KBUILD_CFLAGS) $(1) -c -x c /dev/null -o "$$TMP",$(1),$(2))

# cc-option-yn
# Usage: flag := $(call cc-option-yn,-march=winchip-c6)
cc-option-yn = $(call try-run,\
	$(CC) $(KBUILD_CPPFLAGS) $(KBUILD_CFLAGS) $(1) -c -x c /dev/null -o "$$TMP",y,n)

# cc-option-align
# Prefix align with either -falign or -malign
cc-option-align = $(subst -functions=0,,\
	$(call cc-option,-falign-functions=0,-malign-functions=0))

# cc-disable-warning
# Usage: cflags-y += $(call cc-disable-warning,unused-but-set-variable)
cc-disable-warning = $(call try-run,\
	$(CC) $(KBUILD_CPPFLAGS) $(KBUILD_CFLAGS) -W$(strip $(1)) -c -x c /dev/null -o "$$TMP",-Wno-$(strip $(1)))

# cc-version
# Usage gcc-ver := $(call cc-version)
cc-version = $(shell $(CONFIG_SHELL) $(srctree)/scripts/gcc-version.sh $(CC))

# cc-fullversion
# Usage gcc-ver := $(call cc-fullversion)
cc-fullversion = $(shell $(CONFIG_SHELL) \
	$(srctree)/scripts/gcc-version.sh -p $(CC))

# cc-ifversion
# Usage:  EXTRA_CFLAGS += $(call cc-ifversion, -lt, 0402, -O1)
cc-ifversion = $(shell [ $(call cc-version, $(CC)) $(1) $(2) ] && echo $(3))

# cc-ldoption
# Usage: ldflags += $(call cc-ldoption, -Wl$(comma)--hash-style=both)
cc-ldoption = $(call try-run,\
	$(CC) $(1) -nostdlib -x c /dev/null -o "$$TMP",$(1),$(2))

# ld-option
# Usage: LDFLAGS += $(call ld-option, -X)
ld-option = $(call try-run,\
	$(CC) /dev/null -c -o "$$TMPO" ; $(LD) $(1) "$$TMPO" -o "$$TMP",$(1),$(2))

# ar-option
# Usage: KBUILD_ARFLAGS := $(call ar-option,D)
# Important: no spaces around options
ar-option = $(call try-run, $(AR) rc$(1) "$$TMP",$(1),$(2))

######

###
# Shorthand for $(Q)$(MAKE) -f tools/make/Makefile.build obj=
# Usage:
# $(Q)$(MAKE) $(build)=dir
build := -f tools/make/build.mk obj

# Prefix -I with $(srctree) if it is not an absolute path.
# skip if -I has no parameter
addtree = $(if $(patsubst -I%,%,$(1)), \
$(if $(filter-out -I/%,$(1)),$(patsubst -I%,-I$(srctree)/%,$(1))) $(1))

# Find all -I options and call addtree
flags = $(foreach o,$($(1)),$(if $(filter -I%,$(o)),$(call addtree,$(o)),$(o)))

# echo command.
# Short version is used, if $(quiet) equals `quiet_', otherwise full one.
echo-cmd = $(if $($(quiet)cmd_$(1)),\
	echo '  $(call escsq,$($(quiet)cmd_$(1)))';)

# printing commands
cmd = @$(echo-cmd) $(cmd_$(1))

cmd_dep = @set -e; $(echo-cmd) $(cmd_$(1)); \
	sed -e 's|.*:|$@:|;s|h$$|h \;|' < $(depfile) > $(dot-target).tmp;    \
	mv -f $(dot-target).tmp $(depfile)

# Add $(obj)/ for paths that are not absolute
objectify = $(foreach o,$(1),$(if $(filter /%,$(o)),$(o),$(obj)/$(o)))

###
# if_changed      - execute command if any prerequisite is newer than
#                   target, or command line has changed
# if_changed_dep  - as if_changed, but uses fixdep to reveal dependencies
#                   including used config symbols
# if_changed_rule - as if_changed but execute rule instead
# See Documentation/kbuild/makefiles.txt for more info

ifneq ($(BUILD_NOCMDDEP),1)
# Check if both arguments has same arguments. Result is empty string if equal.
# User may override this check using make BUILD_NOCMDDEP=1
arg-check = $(strip $(filter-out $(cmd_$(1)), $(cmd_$@)) \
                    $(filter-out $(cmd_$@), $(cmd_$(1))) )
else
arg-check = $(if $(strip $(cmd_$@)),,1)
endif

# >'< substitution is for echo to work,
# >$< substitution to preserve $ when reloading .cmd file
# note: when using inline perl scripts [perl -e '...$$t=1;...']
# in $(cmd_xxx) double $$ your perl vars
make-cmd = $(subst \\,\\\\,$(subst \#,\\\#,$(subst $$,$$$$,$(call escsq,$(cmd_$(1))))))

# Find any prerequisites that is newer than target or that does not exist.
# PHONY targets skipped in both cases.
any-prereq = $(filter-out $(PHONY),$?) $(filter-out $(PHONY) $(wildcard $^),$^)

# Execute command if command has changed or prerequisite(s) are updated.
#
if_changed = $(if $(strip $(any-prereq) $(arg-check)),                       \
	@set -e;                                                             \
	$(echo-cmd) $(cmd_$(1));                                             \
	echo 'cmd_$@ := $(make-cmd)' > $(dot-target).cmd)

# Execute the command and also postprocess generated .d dependencies file.
if_changed_dep = $(if $(strip $(any-prereq) $(arg-check) ),                  \
	@set -e;                                                             \
	$(echo-cmd) $(cmd_$(1)); \
	echo 'cmd_$@ := $(make-cmd)\n' > $(dot-target).tmp;                  \
	echo 'source_$@ := $<\n' >> $(dot-target).tmp; \
	sed -e 's|.*: $<|deps_$@ := \\\n|' < $(depfile) >> $(dot-target).tmp;\
	sed -i -e 's|^ |    |' $(dot-target).tmp;                            \
	echo '\n$@: $$(deps_$@)' >> $(dot-target).tmp;                       \
	echo '\n$$(deps_$@):' >> $(dot-target).tmp;                          \
	rm -f $(depfile);                                                    \
	mv -f $(dot-target).tmp $(dot-target).cmd)

# Usage: $(call if_changed_rule,foo)
# Will check if $(cmd_foo) or any of the prerequisites changed,
# and if so will execute $(rule_foo).
if_changed_rule = $(if $(strip $(any-prereq) $(arg-check) ),                 \
	@set -e;                                                             \
	$(rule_$(1)))
