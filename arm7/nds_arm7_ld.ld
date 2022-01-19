/*
			Copyright (C) 2017  Coto
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA
*/

/* Coto: linker file for NDS ARM7 processor, could be adapted for DSi-3DS */

/* attributes can actually help in debugging because if you try to move functions to DTCM, for example, compiler will rant since DTCM is DATA only path and not execution*/
MEMORY
{
	IWRAM	(rwx)	: ORIGIN = 0x03800000, LENGTH = 64K
}

_iwram_start = ORIGIN(IWRAM);
_iwram_end = ORIGIN(IWRAM)	+	LENGTH(IWRAM);

/* ARM7 Specific */
_arm7_irqcheckbits = 0x0380FFF8;	/* __irq_flags */
_arm7_irqhandler = 0x0380FFFC;	/* __irq_vector */

/* ARM ABI says stacks are full decreasing */
sp_SVC	= _iwram_end - 0x200;
sp_IRQ	= sp_SVC - 0x200;
sp_FIQ	= sp_IRQ - 0x200;
sp_ABT	= sp_FIQ - 0x200;
sp_UND	= sp_ABT - 0x200;
sp_USR	= sp_UND - 0x200;
sp_SYS	= sp_USR;

OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm","elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	.init      :	SUBALIGN(4)
	{
		KEEP(*(.init))
	} > IWRAM = 0xFF
	
	. = ALIGN(4);
	
	.dldiSection      :	SUBALIGN(4)
	{
		KEEP(*(.initDldiSection))
		KEEP(*(.dldiSection))
	} > IWRAM = 0xFF
	
	. = ALIGN(4);
	
	.text      :	SUBALIGN(4)
	{
		KEEP(*(.stub))
		/* .gnu.warning sections are handled specially by elf32.em.  */
		KEEP(*(.gnu.warning))
		KEEP(*(.gnu.linkonce.t*))
		KEEP(*(.glue_7t)) 
		KEEP(*(.glue_7))
		KEEP(*(COMMON))
		KEEP(*(.text))
		KEEP(*(.text.*))
	} > IWRAM = 0xFF
	
	. = ALIGN(4);
	
	.data : SUBALIGN(4) {				/* used by TGDS-Newlib C/C++ code */
		PROVIDE (__data_start__ = ABSOLUTE(.));
		KEEP(*(.data))
		KEEP(*(.data.*))
		KEEP(*(.gnu.linkonce.d*))
		KEEP(*(.gnu.linkonce.t*))
		PROVIDE (__data_end__ = ABSOLUTE(.));
	} > IWRAM = 0xff
	
	. = ALIGN(4);
	
	.rodata : SUBALIGN(4) {			/* used by TGDS-Newlib C/C++ code */
		PROVIDE (__rodata_start__ = ABSOLUTE(.));
		KEEP(*(.rdata))
		KEEP(*(.rodata))
		KEEP(*(.rodata.*))
    	KEEP(*(.gnu.linkonce.r.*))
		PROVIDE (__rodata_end__ = ABSOLUTE(.));
	} > IWRAM = 0xff
	
	. = ALIGN(4);
	
	.v4_bx : SUBALIGN(4) {
		PROVIDE (__v4_bx_start__ = ABSOLUTE(.));
		*(.v4_bx*)
		PROVIDE (__v4_bx_end__ = ABSOLUTE(.));
	} > IWRAM = 0xff
	
	. = ALIGN(4);
	
	.iplt : SUBALIGN(4) {
		PROVIDE (__iplt_start__ = ABSOLUTE(.));
		*(.iplt*)
		PROVIDE (__iplt_end__ = ABSOLUTE(.));
	} > IWRAM = 0xff
	
	. = ALIGN(4);
	
	.igot.plt : SUBALIGN(4) {
		PROVIDE (__igot_plt_start__ = ABSOLUTE(.));
		*(.igot.plt*)
		PROVIDE (__igot_plt_end__ = ABSOLUTE(.));
	} > IWRAM = 0xff
	
	. = ALIGN(4);
	
	.rel.dyn : SUBALIGN(4) {
		PROVIDE (__rel_dyn_start__ = ABSOLUTE(.));
		*(.rel.dyn*)
		PROVIDE (__rel_dyn_end__ = ABSOLUTE(.));
	} > IWRAM = 0xff
	
	. = ALIGN(4);
	
	.bss :	SUBALIGN(4) {				/* used by TGDS-Newlib C/C++ code */
		PROVIDE (__bss_vma_start = ABSOLUTE(.));
		KEEP(*(.dynbss*))
		KEEP(*(.bss*))
		KEEP(*(.bss))
		KEEP(*(.gnu.linkonce.b*.))
		PROVIDE (__bss_vma_end = ABSOLUTE(.));
	} > IWRAM
	
	. = ALIGN(4);
	
	.ARM.exidx : SUBALIGN(4) {			/* not used at all, but some code rely on ARM debugging symbols */
		PROVIDE (__exidx_start = ABSOLUTE(.));
		KEEP(*(.ARM.exidx))
		KEEP(*(.ARM.exidx*))
		KEEP(*(.gnu.linkonce.armexidx.))
		KEEP(*(.gnu.linkonce.armexidx.*))
		PROVIDE (__exidx_end = ABSOLUTE(.));
	} > IWRAM = 0xff
	
	. = ALIGN(4);
	
	.fini : SUBALIGN(4) {				/* used by TGDS-Newlib C/C++ code */
		PROVIDE (__fini_start__ = ABSOLUTE(.));
		*(.fini)
		*(.fini*)
		PROVIDE (__fini_end__ = ABSOLUTE(.));
	} > IWRAM = 0xff
	
	PROVIDE (__lib__end__ = __fini_end__);
	. = ALIGN(0x40);
	PROVIDE (__vma_stub_end__ = __lib__end__ + 0x40);
	
	/* stubbed for ARM7i binaries crt0 in NTR mode, but actually used in TWL mode */
	__arm7i_lma__ = __vma_stub_end__ + 4;
	__arm7i_start__ = __arm7i_lma__ + 4;
	__arm7i_end__ = __arm7i_start__ + 4;
	
	__arm7_lma__ = __arm7i_end__ + 4;
	__arm7_start__ = __arm7_lma__ + 4;
	__arm7_end__ = __arm7_start__ + 4;
	
	/*
     	* Stabs le debugging sections.
     	*
     	*/

    	.stab          0 : { *(.stab) }
    	.stabstr       0 : { *(.stabstr) }
    	.stab.excl     0 : { *(.stab.excl) }
    	.stab.exclstr  0 : { *(.stab.exclstr) }
    	.stab.index    0 : { *(.stab.index) }
    	.stab.indexstr 0 : { *(.stab.indexstr) }
    	.comment       0 : { *(.comment) }
    	/* DWARF debug sections.
       	Symbols in the DWARF debugging sections are relative to the beginning
       	of the section so we begin them at 0.  */
    	/* DWARF 1 */
    	.debug          0 : { *(.debug) }
    	.line           0 : { *(.line) }
    	/* GNU DWARF 1 extensions */
    	.debug_srcinfo  0 : { *(.debug_srcinfo) }
    	.debug_sfnames  0 : { *(.debug_sfnames) }
    	/* DWARF 1.1 and DWARF 2 */
    	.debug_aranges  0 : { *(.debug_aranges) }
    	.debug_pubnames 0 : { *(.debug_pubnames) }
    	/* DWARF 2 */
    	.debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    	.debug_abbrev   0 : { *(.debug_abbrev) }
    	.debug_line     0 : { *(.debug_line) }
    	.debug_frame    0 : { *(.debug_frame) }
    	.debug_str      0 : { *(.debug_str) }
    	.debug_loc      0 : { *(.debug_loc) }
    	.debug_macinfo  0 : { *(.debug_macinfo) }
    	/* SGI/MIPS DWARF 2 extensions */
    	.debug_weaknames 0 : { *(.debug_weaknames) }
    	.debug_funcnames 0 : { *(.debug_funcnames) }
    	.debug_typenames 0 : { *(.debug_typenames) }
    	.debug_varnames  0 : { *(.debug_varnames) }
	
}