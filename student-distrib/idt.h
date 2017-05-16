/* idt.h - Initializes the IDT and handles exceptions */

#ifndef IDT_FILE_H
#define IDT_FILE_H



/* Define local constants */


#define TOTAL_DEFINED_EXCEPTIONS 19
#define TOTAL_EXCEPTIONS 32
#define SYSTEM_CALL_INDEX 0x80


#ifndef ASM

/* Initialize the Interrupt Descriptor Table */
void initialize_idt(void);
extern void handle_keyboard(); 
void system_call_handler(int syscall_idx);

#endif /* ASM */

#endif /* IDT_H */
