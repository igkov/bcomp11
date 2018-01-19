/***********************************************************************/
/*                                                                     */
/*  ROM.ld:  Linker Script File                                        */
/*                                                                     */
/***********************************************************************/
ENTRY(_startup)
STACK_SIZE = 0x400;

/* Memory map */
MEMORY
{
    ROM (rx) : ORIGIN = 0x00000000, LENGTH = 0x00008000
    RAM (rw) : ORIGIN = 0x10000000, LENGTH = 0x00001F00
}

SECTIONS                                    /* Section Definitions */
{
    . = ALIGN(4);
    .text :                                 /* first section is .text which is used for code */
    {
        *startup_LPC11xx_gcc.o (.text)                  /* Startup code */
        *(.text)                            /* remaining code */
    } > ROM
    
    . = ALIGN(4);
    .rodata :
    {
        *(.rodata)                          /* all .rodata sections (constants, strings, etc.)  */
        *(.rodata*)                         /* all .rodata* sections (constants, strings, etc.)  */
        *(.glue_7)                          /* all .glue_7 sections  (no idea what these are) */
        *(.glue_7t)                         /* all .glue_7t sections (no idea what these are) */
    } > ROM

    . = ALIGN(4);
    
    /* .ctors .dtors are used for c++ constructors/destructors */
    .ctors :
    {
        PROVIDE(__ctors_start__ = .);
        KEEP(*(SORT(.ctors.*)))
        KEEP(*(.ctors))
        PROVIDE(__ctors_end__ = .);
    } >ROM
    
    .dtors :
    {
        PROVIDE(__dtors_start__ = .); 
        KEEP(*(SORT(.dtors.*)))
        KEEP(*(.dtors))
        PROVIDE(__dtors_end__ = .);
    } >ROM
    
    . = ALIGN(4);
    _etext = . ;
    PROVIDE (etext = .);
    .data : AT (_etext)                     /* .data section which is used for initialized data */
    {
        _data = . ;
        *(.data)
        SORT(CONSTRUCTORS)
    } > RAM
    
    . = ALIGN(4);
    _edata = . ;
    PROVIDE (edata = .);  
    
    .bss (NOLOAD):                        /* .bss section which is used for uninitialized data */
    {
        *(.noinit)
        . = ALIGN(4);
        __bss_start = . ;
        __bss_start__ = . ;
        *(.bss)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = . ;
    } > RAM
    
    .stack ALIGN(256) :
    {
        . += STACK_SIZE;
        PROVIDE (_stack = .);
    } > RAM
    
    . = ALIGN(4);
    _end = . ;
    PROVIDE (end = .);
    
    /* Stabs debugging sections.  */
    .stab          0 : { *(.stab) }
    .stabstr       0 : { *(.stabstr) }
    .stab.excl     0 : { *(.stab.excl) }
    .stab.exclstr  0 : { *(.stab.exclstr) }
    .stab.index    0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment       0 : { *(.comment) }
    /* 
        DWARF debug sections.
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
