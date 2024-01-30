ENTRY(start)

SECTIONS {
        . = 1M;

        .boot :
        {
                /* multiboot header is at the beginning */
                *(.multiboot_header)
        }

        .text :
        {
                *(.text)
        }
}
