ENTRY(_start)

SECTIONS
{
    . = 0xC0000000;

    .text ALIGN(4K) : AT(ADDR(.text) - 0xBFF00000)
    {
        *(.multiboot)
        *(.text)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xBFF00000)
    {
        *(.rodata)
    }

    .data ALIGN(4K) : AT(ADDR(.data) - 0xBFF00000)
    {
        *(.data)
    }

    .bss ALIGN(4K) : AT(ADDR(.bss) - 0xBFF00000)
    {
        *(COMMON)
        *(.bss)
    }

    end = .; _end = .; __end = .;

}
