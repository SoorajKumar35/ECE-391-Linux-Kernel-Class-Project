#include "i8259.h"
#include "lib.h"
#include "paging_handler.h"
#include "scheduler.h"
#include "syscalls.h"
#include "types.h"
#include "terminal_driver.h"
#include "x86_desc.h"

uint32_t esp_store;
uint32_t ebp_store;
uint32_t currently_running;

void init_PIT(void){

	//not sure but divided by 20 and not 20000, TA said
	int divisor = 1193180 / 20;
	//set time phase
	outb(0x36, 0x43);
	/* Set low byte of divisor */
    outb(divisor & 0xFF, 0x40);
    /* Set high byte of divisor */
    outb(divisor >> 8, 0x40);
	
	//enable irq 0
	enable_irq(0);

}


void inter_PIT(void){

	static int counter = 0;
	cli();
	send_eoi(0);
	// printf("hey man\n");

	//The PIT interrupt happens so we checked whether the next terminal has been opened and if it has we switch 
	//to the currently executing process in that terminal
	if(terms[(counter +1)%3].been_here == 1)
	{
		while(1)
			printf("We are in first if else in inter PIT\n");
		switch_active_task((counter +1)%3);
		// currently_running = current_terminal;
		// switch_to_other_term((current_terminal +1)%3);
		counter++;

	}
	else if(terms[(counter +2)%3].been_here == 1)
	{
		while(1)
			printf("We are in second if else in inter PIT\n");
		switch_active_task((counter +2)%3);
		// switch_to_other_term((current_terminal +2)%3);
		counter++;

	}	
	else if(terms[(counter +3)%3].been_here == 1)
	{
			// counter++;
		// printf("No other open terminals\n");
		// switch_active_task((counter +3)%3);
		// switch_to_other_term((current_terminal +3)%3);
	}
	else
	{
		printf("shunu hada\n");
	}
	//It will always get back here after we switch the tasks so we are good yay!
	// counter++;
	return;
}

void switch_active_task(int term)
{
	asm("movl %%esp, %%eax ;"
		"movl %%eax, esp_store ;"
    	"movl %%ebp, %%eax; "
    	"movl %%eax, ebp_store ;"
    	: : :  "ebp", "esp");
    terms[currently_running].current_esp = (uint32_t *)esp_store;
    terms[currently_running].current_ebp = (uint32_t *)ebp_store;
    terms[currently_running].esp0 = tss.esp0;

    currently_running = term;

	esp_store = (uint32_t)terms[currently_running].current_esp;
	ebp_store = (uint32_t)terms[currently_running].current_ebp;
    load_new_process(terms[currently_running].running_pcb->process_num);
	// printf("esp store %x\n",esp_store );
	// printf("ebp store %x\n",ebp_store);
	tss.esp0 = terms[currently_running].esp0;
	asm("movl esp_store, %%esp ;"
    	"movl ebp_store, %%ebp ;"
    : : :  "ebp", "esp");	

}


















