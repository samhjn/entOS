ENTRY(start)
phys = 0x200000;
SECTIONS
{
  .text phys : AT(phys) 
  {
	code = .;
	*(.code)
	. = ALIGN(4);
    *(.text)
    *(.rodata)
    . = ALIGN(4);
  }
  .data : AT(phys + (data - code))
  {
    data = .;
    *(.data)
    . = ALIGN(4);
  }
  .bss : AT(phys + (bss - code))
  {
    bss = .;
    *(.bss)
    . = ALIGN(4);
  }
  _end = .;
}