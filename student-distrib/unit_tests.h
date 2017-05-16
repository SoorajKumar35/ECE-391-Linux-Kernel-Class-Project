#ifndef UNIT_TESTS_H
#define UNIT_TESTS_H

#ifndef ASM 


void test_idt();
void test_paging_handler();
void test_terminal_driver(); 
extern void rtc_test(); 
extern int rtc_interrupt_happened; 

#endif /* ASM */
 
#endif /* UNIT_TESTS_H */ 
