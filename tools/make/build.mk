# ==========================================================================
# Building
# ==========================================================================

src := $(obj)

PHONY := __build
__build:

# Init all relevant variables so they do not
# inherit any value from the environment
obj-y :=
targets :=
subdir-y :=
asflags-y  :=
ccflags-y  :=
cppflags-y :=
ldflags-y  :=

-include $(BUILD_CONFIG)
include tools/make/include.mk
include $(src)/Makefile
include tools/make/lib.mk

#ifneq ($(strip $(obj-y) $(obj-n) $(obj-)),)
ifneq ($(src),.)
builtin-target := $(obj)/built-in.o
endif

__build: $(builtin-target) $(extra-y) $(subdir-y)
	@:

# Compile C sources (.c)
# ---------------------------------------------------------------------------

quiet_cmd_cc_s_c = CC      $@
cmd_cc_s_c       = $(CC) $(c_flags) -fverbose-asm -S -o $@ $<

$(obj)/%.s: $(src)/%.c FORCE
	$(call if_changed_dep,cc_s_c)

quiet_cmd_cc_i_c = CPP     $@
cmd_cc_i_c       = $(CPP) $(c_flags)   -o $@ $<

$(obj)/%.i: $(src)/%.c FORCE
	$(call if_changed_dep,cc_i_c)

cmd_gensymtypes =                                                           \
    $(CPP) -D__GENKSYMS__ $(c_flags) $< |                                   \
    $(GENKSYMS) $(if $(1), -T $(2)) -a $(ARCH)                              \
     $(if $(KBUILD_PRESERVE),-p)                                            \
     -r $(firstword $(wildcard $(2:.symtypes=.symref) /dev/null))

quiet_cmd_cc_symtypes_c = SYM     $@
cmd_cc_symtypes_c =                                                         \
    set -e;                                                                 \
    $(call cmd_gensymtypes,true,$@) >/dev/null;                             \
    test -s $@ || rm -f $@

$(obj)/%.symtypes : $(src)/%.c FORCE
	$(call cmd,cc_symtypes_c)

quiet_cmd_cc_o_c = CC      $@
      cmd_cc_o_c = $(CC) $(c_flags) -c -o $@ $<

$(obj)/%.o: $(src)/%.c FORCE
	$(call if_changed_dep,cc_o_c)

# Compile assembler sources (.S)
# ---------------------------------------------------------------------------

quiet_cmd_as_s_S = CPP    $@
      cmd_as_s_S = $(CPP) $(a_flags) -o $@ $< 

$(obj)/%.s: $(src)/%.S FORCE
	$(call if_changed_dep,as_s_S)

quiet_cmd_as_o_S = AS      $@
      cmd_as_o_S = $(CC) $(a_flags) -c -o $@ $<

$(obj)/%.o: $(src)/%.S FORCE
	$(call if_changed_dep,as_o_S)

targets += $(obj-y) $(extra-y) $(MAKECMDGOALS)

# Linker scripts preprocessor (.lds.S -> .lds)
# ---------------------------------------------------------------------------
quiet_cmd_cpp_lds_S = LDS     $@
      cmd_cpp_lds_S = $(CPP) $(cpp_flags) -P -C -U$(ARCH) \
	                     -D__ASSEMBLY__ -DLINKER_SCRIPT -o $@ $<

$(obj)/%.lds: $(src)/%.lds.S FORCE
	$(call if_changed_dep,cpp_lds_S)

# Build the compiled-in targets
# ---------------------------------------------------------------------------

# To build objects in subdirs, we need to descend into the directories
$(sort $(subdir-obj-y)): $(subdir-y) ;

#
# Rule to compile a set of .o files into one .o file
#
ifdef builtin-target
quiet_cmd_link_o_target = LD      $@
# If the list of objects to link is empty, just create an empty built-in.o
cmd_link_o_target = $(if $(strip $(obj-y)),\
		      $(LD) $(ld_flags) -r -o $@ $(filter $(obj-y), $^), \
		      rm -f $@; $(AR) rcs$(ARFLAGS) $@)

$(builtin-target): $(obj-y) FORCE
	$(call if_changed,link_o_target)

targets += $(builtin-target)
endif

# Descending
# ---------------------------------------------------------------------------

PHONY += $(subdir-y)
$(subdir-y):
	$(Q)$(MAKE) $(build)=$@

# Add FORCE to the prequisites of a target to force it to be always rebuilt.
# ---------------------------------------------------------------------------

PHONY += FORCE
FORCE:

# Read all saved command lines and dependencies for the $(targets) we
# may be building above, using $(if_changed{,_dep}). As an
# optimization, we don't need to read them if the target does not
# exist, we will rebuild anyway in that case.

targets := $(wildcard $(sort $(targets)))
cmd_files := $(wildcard $(foreach f,$(targets),$(dir $(f)).$(notdir $(f)).cmd))

ifneq ($(cmd_files),)
  include $(cmd_files)
endif

# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable se we can use it in if_changed and friends.

.PHONY: $(PHONY)
