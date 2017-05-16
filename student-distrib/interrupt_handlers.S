#define ASM 1 
#include "idt.h"

.global keyboard_handler 

keyboard_handler: 
	pushal 
	pushfl 
	call handle_keyboard 
	popfl
	popal 
	iret 

.global test_interrupts_handler

test_interrupts_handler: 
	pushal 
	pushfl 
	call test_interrupts
	popfl 
	popal 
	iret 

.global rtc_test_handler
	rtc_test_handler: 
	pushal 
	pushfl 
	call rtc_test
	popfl 
	popal 
	iret 
