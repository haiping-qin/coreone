plat-$(CONFIG_PLAT_VERSATILE)	+= versatile
mach-$(CONFIG_PLAT_VERSATILE)	+= versatile

platdirs := $(patsubst %,arch/arm/plat-%/,$(plat-y))
machdirs := $(patsubst %,arch/arm/mach-%/,$(mach-y))

CPPFLAGS += $(patsubst %,-I%include,$(platdirs) $(machdirs))

core-y				+= arch/arm/kernel/
core-y				+= $(machdirs) $(platdirs)
libs-y				:= arch/arm/lib/ $(libs-y)

ifneq ($(CONFIG_ARM_CPU),)
CFLAGS		+= -mcpu=$(CONFIG_ARM_CPU)
endif
ifneq ($(CONFIG_ARM_ARCH),)
CFLAGS		+= -march=$(CONFIG_ARM_ARCH)
endif

ifeq ($(CONFIG_ARM_THUMB),y)
CFLAGS		+= -mthumb -D__thumb__
ifeq ($(findstring eabi-,$(CONFIG_CROSS_COMPILE)),)
CFLAGS		+= -mthumb-interwork
endif
endif

LDFLAGS_one	+= -gc-sections
LDFLAGS_one	+= $(shell $(CC) -print-libgcc-file-name)
