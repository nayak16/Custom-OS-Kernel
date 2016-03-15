/** @file install_handlers.c
 *  @brief implements installation of syscall handlers and peripheral handlers
 *
 *  @author Christopher Wei (cjwei)
 *  @author Aatish Nayak (aatishn)
 *  @bug No known bugs.
 */


#include <stdint.h>

/* some common byte masks and offsets */
#include <constants.h>
#include <syscall_int.h>

/* idt_base() */
#include <x86/asm.h>
#include <x86/seg.h>
#include <x86/idt.h>
#include <x86/timer_defines.h>
#include <x86/keyhelp.h>

#include <idt_handlers.h>
/**
 * CONSTANTS
 */

/** @brief desired period between clock interrupts in milliseconds */
#define TIMER_PERIOD_MS 10
/** @brief number of milliseconds per second */
#define C_MS_PER_SEC 1000
/** @brief the number of clock cycles to wait per interrupt */
#define TIMER_CYCLES ((TIMER_RATE * TIMER_PERIOD_MS) / C_MS_PER_SEC)

/** @brief (2 byte) offset of lower 2 bytes of handler from idt entry */
#define LOFFSET_OFFSET 0
/** @brief (2 byte) offset of segement selector from idt entry */
#define SEGSEL_OFFSET 1
/** @brief (2 byte) offset of the flags from idt entry */
#define FLAG_OFFSET 2
/** @brief (2 byte) offset of upper 2 bytes of handler from idt entry */
#define HOFFSET_OFFSET 3

/** @brief denotes present flag is True */
#define FLAG_PRESENT_TRUE 1
/** @brief (bit) offset of present flag */
#define FLAG_PRESENT_OFFSET 7

/** @brief denotes size is 32 bits */
#define FLAG_D_32 1
/** @brief (bit) offset of size flag */
#define FLAG_D_OFFSET 3

/** @brief denotes 0 privilege level */
#define FLAG_DPL_0 0
#define FLAG_DPL_3 3
/** @brief (bit) offset of privilege flag */
#define FLAG_DPL_OFFSET 5

/** @brief (4 bytes) size of a IDT entry  */
#define IDT_ENTRY_WORD_WIDTH 2

/** @brief the base flags of a trap gate */
#define FLAG_TRAP_GATE 0x7
/** @brief the base flags of a interrupt gate */
#define FLAG_INTERRUPT_GATE 0x6

/** @brief bit mask used to clear top byte of 2 byte int*/
#define FLAG_RESET_MASK 0x0F


/** @brief installs an entry in the IDT
 *
 *  @param offset The offset from the base address that should be used as a
 *         handler for this interrupt
 *  @param seg_sel The selected segment to use
 *  @param p The present flag
 *  @param dpl The privilege level flag
 *  @param d The size flag
 *  @param entryid The numeric offset from the 0th idt entry
 *  @return Void
 */
void idt_install_entry(uint32_t offset, uint16_t seg_sel, uint8_t p,
    uint8_t dpl, uint8_t d, uint32_t entryid, uint16_t flag_base){
    uint16_t offset_l, offset_h, flags;
    uint16_t *idt_entry;
    idt_entry = (uint16_t *)(((uint32_t *)idt_base()) +
            IDT_ENTRY_WORD_WIDTH * entryid);
    offset_l = (uint16_t)(offset & C_L2B_MASK);
    offset_h = (uint16_t)((offset >> C_2BYTE_WIDTH) & C_L2B_MASK);
    /* assemble flag byte */
    flags = (flag_base | (p << FLAG_PRESENT_OFFSET)
        | (dpl << FLAG_DPL_OFFSET) | (d << FLAG_D_OFFSET)) << C_BYTE_WIDTH;

    /* save lower offset */
    *(idt_entry + LOFFSET_OFFSET) = offset_l;
    /* save segment selector */
    *(idt_entry + SEGSEL_OFFSET) = seg_sel;
    /* clear current upper flag byte */
    *(idt_entry + FLAG_OFFSET) &= FLAG_RESET_MASK;
    /* save flag byte */
    *(idt_entry + FLAG_OFFSET) |= flags;
    /* save upper offset */
    *(idt_entry + HOFFSET_OFFSET) = offset_h;
}

/* Implementation */

int install_syscall_handlers(){
    idt_install_entry((uint32_t)syscall_gettid_handler, SEGSEL_KERNEL_CS,
        FLAG_PRESENT_TRUE, FLAG_DPL_3, FLAG_D_32, GETTID_INT, FLAG_TRAP_GATE);
    return 0;
}

int install_exception_handlers() {

    /* Page Fault */
    idt_install_entry((uint32_t) page_fault_handler, SEGSEL_KERNEL_CS,
        FLAG_PRESENT_TRUE, FLAG_DPL_0, FLAG_D_32, IDT_PF, FLAG_INTERRUPT_GATE);

    /* Double Fault */
    idt_install_entry((uint32_t) double_fault_handler, SEGSEL_KERNEL_CS,
        FLAG_PRESENT_TRUE, FLAG_DPL_0, FLAG_D_32, IDT_DF, FLAG_INTERRUPT_GATE);

    return 0;
}

int install_peripheral_handlers(){
    char lsb, msb;

    /* set tickback */
    //timer_callback = tickback;

    /* install IDT entry for timer*/

    idt_install_entry((uint32_t)timer_handler, SEGSEL_KERNEL_CS,
        FLAG_PRESENT_TRUE, FLAG_DPL_0, FLAG_D_32, TIMER_IDT_ENTRY, FLAG_INTERRUPT_GATE);

    /* setup timer */

    outb(TIMER_SQUARE_WAVE, TIMER_MODE_IO_PORT);
    lsb = TIMER_CYCLES & C_BYTE_MASK;
    msb = (TIMER_CYCLES >> C_BYTE_WIDTH) & C_BYTE_MASK;
    outb(lsb, TIMER_PERIOD_IO_PORT);
    outb(msb, TIMER_PERIOD_IO_PORT);

    /* install IDT entry for keyboard */
    idt_install_entry((uint32_t)keyboard_handler, SEGSEL_KERNEL_CS,
        FLAG_PRESENT_TRUE, FLAG_DPL_0, FLAG_D_32, KEY_IDT_ENTRY, FLAG_INTERRUPT_GATE);

    return 0;
}
