/* --- linker.ld --- */
/* The entry point defined in boot.s */
ENTRY(_start)

SECTIONS
{
	/* The kernel will be loaded at 1 MiB (0x100000) */
	. = 1M;

	/* First we put the Multiboot header at the top of the binary */
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)
		*(.text)
	}

	/* Read-only data (strings, constants, etc) */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

	/* Initialized read-write data */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	/* Uninitialized data (BSS) and Stack space */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}

	/* The compiler ensures sections are aligned to 4K (4096 bytes) */
}
