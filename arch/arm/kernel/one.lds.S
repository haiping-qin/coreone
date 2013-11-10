OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(_start)
SECTIONS
{
	. = MEMBASE;

	.text :	{ *(.text .text.* .glue_7* .gnu.linkonce.t.*) } = 0x9090

	.rodata : { 
		*(.rodata .rodata.* .gnu.linkonce.r.*)
		. = ALIGN(4);
		__commands_start = .;
		KEEP (*(.commands))
		__commands_end = .;
		. = ALIGN(4);
		__apps_start = .;
		KEEP (*(.apps))
		__apps_end = .;
		. = ALIGN(4); 
		__rodata_end = . ;		
	}

	/* writable data  */

	/* in one segment binaries, the rom data address is on top of the ram data address */
	__data_start_rom = .;

	__data_start = .;
	.data : SUBALIGN(4) { *(.data .data.* .gnu.linkonce.d.*) }
	__data_end = .;

	/* unintialized data (in same segment as writable data) */
	. = ALIGN(4);
	__bss_start = .;
	.bss : { *(.bss .bss.*) }

	. = ALIGN(4); 
	_end = .;

	. = MEMBASE + MEMSIZE;
	_end_of_ram = .;

	/* Strip unnecessary stuff */
	/DISCARD/ : { *(.comment .note .eh_frame) }
}
