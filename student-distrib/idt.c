
/*  FILE:		idt.c
 *  PROGRAMMER: Griffin A. Tucker
 *  DATE:		March 11 2017
 *  PURPOSE:	Supplies functions to initialize IDT and respond 
 *              to exceptions
 */

#include "idt.h"
#include "x86_desc.h"
#include "interrupt_handlers.h"
#include "i8259.h"
#include "terminal_driver.h"
#include "lib.h"
#include "keyboard_driver.h"
#include "syscall_handler.h"

uint32_t cr2_store;

/* 
 REFERENCE (SCANCODE TO ASCII TABLE) 
	http://www.ee.bgu.ac.il/~microlab/MicroLab/Labs/ScanCodes.htm
*/ 

/* Local functions Prototypes -- see function headers for details */
static void undefined_interrupt();

/* Function prototypes of exception handlers
   Note: Due to the general nature of these prototypes, a general
   function interface has been provided below
*/

static void EXCEPTION_divide_by_zero();
static void EXCEPTION_debug_exception();
static void EXCEPTION_non_maskable_interrupt();
static void EXCEPTION_breakpoint();
static void EXCEPTION_overflow();
static void EXCEPTION_bound_range_exceeded();
static void EXCEPTION_invalid_opcode();
static void EXCEPTION_device_not_available();
static void EXCEPTION_double_fault();
static void EXCEPTION_coprocessor_segment_overrun();
static void EXCEPTION_invalid_TSS();
static void EXCEPTION_stack_segment_fault();
static void EXCEPTION_segment_not_present();
static void EXCEPTION_general_protection_fault();
static void EXCEPTION_page_fault();
static void EXCEPTION_floating_point();
static void EXCEPTION_alignment_check();
static void EXCEPTION_machine_check();
static void EXCEPTION_SIMD_floating_point();


/* FUNCTION: 	EXCEPTION_exception_name
   PROGRAMMER: 	Griffin A. Tucker
   DATE:		March 14 2016
   PURPOSE: 	These functions are called when an exception is fired.
            	They display an error message and loop indefinitely.
   INPUTS: 		Void
   OUTPUTS: 	Void
   RESULTS: 	Prints an error message to the terminal and spins
*/

static void EXCEPTION_divide_by_zero()
{
	cli();
	printf("%s\n", "Divide by zero exception.");
	sti();
	while(1);
}
static void EXCEPTION_debug_exception()
{
	cli();
	printf("%s\n", "Debug exception.");
	sti();
	while(1);
}
static void EXCEPTION_non_maskable_interrupt()
{
	cli();
	printf("%s\n", "Non-maskable interrupt.");
	sti();
	while(1);
}
static void EXCEPTION_breakpoint()
{
	cli();
	printf("%s\n", "Breakpoint exception.");
	sti();
	while(1);
}
static void EXCEPTION_overflow()
{
	cli();
	printf("%s\n", "Overflow exception.");
	sti();
	while(1);
}
static void EXCEPTION_bound_range_exceeded()
{
	cli();
	printf("%s\n", "Bound range exceeded.");
	sti();
	while(1);
}
static void EXCEPTION_invalid_opcode()
{
	cli();
	printf("%s\n", "Invalid opcode.");
	sti();
	while(1);
}
static void EXCEPTION_device_not_available()
{
	cli();
	printf("%s\n", "Device not available.");
	sti();
	while(1);
}
static void EXCEPTION_double_fault()
{
	cli();
	printf("%s\n", "Double fault.");
	sti();
	while(1);
}
static void EXCEPTION_coprocessor_segment_overrun()
{
	cli();
	printf("%s\n", "Coprocessor segment overrun.");
	sti();
	while(1);
}
static void EXCEPTION_invalid_TSS()
{
	cli();
	printf("%s\n", "Invalid TSS.");
	sti();
	while(1);
}
static void EXCEPTION_stack_segment_fault()
{
	cli();
	printf("%s\n", "Stack segment fault.");
	sti();
	while(1);
}
static void EXCEPTION_segment_not_present()
{
	cli();
	printf("%s\n", "Segment not present.");
	sti();
	while(1);
}
static void EXCEPTION_general_protection_fault()
{
	cli();
	printf("%s\n", "General protection fault.");
	sti();
	while(1);
}
static void EXCEPTION_page_fault()
{
	cli();
	asm("movl %%cr2, %%eax;"
		"movl %%eax, cr2_store ;"
		: : : "cc");
	printf("CR2: %x\n", cr2_store );
	printf("%s\n", "Page fault.");
	sti();
	while(1);
}
static void EXCEPTION_floating_point()
{
	cli();
	printf("%s\n", "Floating point exception.");
	sti();
	while(1);
}
static void EXCEPTION_alignment_check()
{
	cli();
	printf("%s\n", "Alignment check exception.");
	sti();
	while(1);
}
static void EXCEPTION_machine_check()
{
	cli();
	printf("%s\n", "Machine check exception.");
	sti();
	while(1);
}
static void EXCEPTION_SIMD_floating_point()
{
	cli();
	printf("%s\n", "SIMD floating point exception.");
	sti();
	while(1);
}

/*  FUNCTION:   undefined_interrupt
 *  PROGRAMMER: Griffin A. Tucker
 *  DATE:		March 11 2017
 *  PURPOSE:	Displays an error message for an undefined interrupt
 *  			and spins indefinitely
 */

static void undefined_interrupt(){
	printf("An undefined interrupt was fired.");
	while(1);
}


void handle_keyboard()
{
	cli();
    update_buffer_and_cursor();
    sti();
}

/*  FUNCTION:	initialize_idt
 *  PROGRAMMER: Griffin A. Tucker
 *  DATE:		March 11 2017
 *  PURPOSE:	Initializes the interrupt descriptor table
 */

void initialize_idt(void)
{ 
	//printf("%d", 1);
	//Index used for traversing idt
	
	int idt_i;	

	// Load the idt register
	lidt(idt_desc_ptr);

	// For every entry in the IDT, initialize a descriptor
	for(idt_i = 0; idt_i < NUM_VEC; idt_i++)
	{
		// Set the interruption vector as present 
		idt[idt_i].present = 0x1;

		// Set priviledge level. 
        // Set to 0 if anything but a system call. 3 otherwise.
		if(idt_i != SYSTEM_CALL_INDEX) idt[idt_i].dpl = 0x0;
		else idt[idt_i].dpl = 0x3;

		// Set reserved bits to default gate
		idt[idt_i].reserved0 = 0x0;
		idt[idt_i].reserved1 = 0x1;
		idt[idt_i].reserved2 = 0x1;
		idt[idt_i].reserved3 = 0x1;
		idt[idt_i].reserved4 = 0x0;
		idt[idt_i].size = 0x1;
	
		// Set segment selector to chip select
		idt[idt_i].seg_selector = KERNEL_CS;

		// If we are initializing exception descriptors...
		// I.E. for entries, 0 - 31
		if(idt_i < TOTAL_EXCEPTIONS || idt_i == 0x28)
		{
			// Set the reserved bit to 0 and the size to 1
			idt[idt_i].reserved0 = 0x0;
		}
		else
		{
			// Otherwise, set the idt entry as undefined
			idt[idt_i].reserved3 = 0x0;
			SET_IDT_ENTRY(idt[idt_i], undefined_interrupt);
		}
	}

	// Set IDT entries 0 - 18 to the defined exceptions
	SET_IDT_ENTRY(idt[0x00], EXCEPTION_divide_by_zero);
    SET_IDT_ENTRY(idt[0x01], EXCEPTION_debug_exception);
    SET_IDT_ENTRY(idt[0x02], EXCEPTION_non_maskable_interrupt);
    SET_IDT_ENTRY(idt[0x03], EXCEPTION_breakpoint);
    SET_IDT_ENTRY(idt[0x04], EXCEPTION_overflow);
    SET_IDT_ENTRY(idt[0x05], EXCEPTION_bound_range_exceeded);
    SET_IDT_ENTRY(idt[0x06], EXCEPTION_invalid_opcode);
	SET_IDT_ENTRY(idt[0x07], EXCEPTION_device_not_available);
	SET_IDT_ENTRY(idt[0x08], EXCEPTION_double_fault);
	SET_IDT_ENTRY(idt[0x09], EXCEPTION_coprocessor_segment_overrun);
	SET_IDT_ENTRY(idt[0x0A], EXCEPTION_invalid_TSS);
    SET_IDT_ENTRY(idt[0x0B], EXCEPTION_stack_segment_fault);
	SET_IDT_ENTRY(idt[0x0C], EXCEPTION_segment_not_present);
	SET_IDT_ENTRY(idt[0x0D], EXCEPTION_general_protection_fault);
	SET_IDT_ENTRY(idt[0x0E], EXCEPTION_page_fault);
	SET_IDT_ENTRY(idt[0x0F], EXCEPTION_floating_point);
    SET_IDT_ENTRY(idt[0x10], EXCEPTION_alignment_check);
	SET_IDT_ENTRY(idt[0x11], EXCEPTION_machine_check);
	SET_IDT_ENTRY(idt[0x12], EXCEPTION_SIMD_floating_point);
	SET_IDT_ENTRY(idt[0x21], keyboard_handler); 
	SET_IDT_ENTRY(idt[0x28], rtc_test_handler);
	SET_IDT_ENTRY(idt[0x80], syscall_handler);
}
