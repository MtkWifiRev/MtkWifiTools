/* made by Edoardo Mantovani, 2025 */
/* simple linker script which merge .text, .rodata and .data into a single section */


SECTIONS
{
        ENTRY(_start)
        .               = 0x0000000;
        .text 0x8000000 : {
                *(.text)
                *(.text.*)
                *(.rodata)
                *(.rodata.*)
                *(.data)
                *(.data.*)

        }

}
