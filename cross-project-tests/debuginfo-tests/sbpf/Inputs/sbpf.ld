SECTIONS
{
  .rodata 0x000000000 : SUBALIGN(8) {
    BYTE(0);
    *(.rodata*)
    *(.data.rel.ro*)
    . = ALIGN(8);
  } :rodata
  .text 0x100000000 : {
     *(.text*)
  } :text
  /DISCARD/ : {
      *(.comment*)
      *(.eh_frame*)
      *(*hash*)
      *(.bss*)
      *(.data*)
      *(.rel.dyn*)
      *(.dynamic)
      *(.dynsym)
      *(.dynstr)
    }
}

PHDRS
{
  rodata PT_LOAD FLAGS(4);
  text PT_LOAD FLAGS(1);
  other PT_NULL FLAGS(0);
}
