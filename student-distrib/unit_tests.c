#include "unit_tests.h"
#include "syscalls.h"
#include "lib.h"
#include "keyboard_driver.h"
#include "paging_handler.h"
#include "i8259.h"

/*  FUNCTION:   test_idt
 *  PROGRAMMER: Griffin A. Tucker
 *  DATE:		March 26 2017
 *  PURPOSE:	Tests idt by attempting to fire exceptions
 *  NOTE: 		UNCOMMENT WHICH SECTION YOU WOULD LIKE TO TEST
 *		  		THE SECOND SECTION IS INTENDED TO BE MODIFIED
 * 		  		BY THE TESTER TO TAKE CARE OF ALL EXCEPTIONS
 */

void test_idt()
{
	/* Fire a divide by zero exception */
	printf("%s", "We are firing a divide by 0 exception.\n");
	int x;
	int y; 
	x = 5;
	y = 0; 
	x = x / y;

	/* Uncomment below and adjust to fire a specific exception*/
	//printf("%s", "We are firing an exception specified by the tester.\n");
	//asm volatile ("int $0x9");
	
}

/*  FUNCTION:   test_paging_handler
 *  PROGRAMMER: Griffin A. Tucker
 *  DATE:		March 26 2017
 *  PURPOSE:	Tests paging by attempting to fire a page fault
 */

void test_paging_handler()
{
	printf("%s", "We will test for paging's enablement. \n We will attempt to access space beyond the scope of the page tables. \n If an exception is fired, things are working as expected.\n");

	// Declare variables
    uint32_t temp_vector; 
    uint32_t temp_address;
    uint32_t table_size_one_over_idx;
    uint32_t max_directory_idx;

    // Declare useful constants
    // the maximum directory entry
    max_directory_idx = 1023;
    // just one out of bounds of a page table
    table_size_one_over_idx = 4095;

    // Create a test vector out of bounds of the memory
    temp_vector =  0x00000000;
	temp_vector &= 0x00000000; // clear the temporary vector
	temp_vector |= 0x00000001; // set the present bit to 1
	temp_address = (uint32_t)&page_table + table_size_one_over_idx;
	temp_vector |= (temp_address); // place the page table address at its specific bit location

	// Attempt to init the faulty vector
	init_new_vector(max_directory_idx, table_size_one_over_idx, temp_vector);
}

void test_terminal_driver() { 
	
	 while(1) {
		 char *str = "Enter something and I will echo it:"; //string we will write on screen 
		 unsigned char buf[LINE_SIZE];  //input buffer for read function 
		 //printf("%d", fd);

		 memset(buf, 0, LINE_SIZE);
	     write(1, str, 35); //35 is length of str 
        // read(0,  buf, LINE_SIZE);
     	// unsigned long i; 
     	// i = 0; 
     	 //while(i < 100000000) i++; //spin for a while 

     	 //clear_screen();  //clear the screen 
     	 //flush(); //flush the input buffer 
     	 
     }

} 

void rtc_test() { 
		rtc_interrupt_happened = 1; 
		//printf("received rtc interrupt");
	    send_eoi(8);
		outb(0x0C, 0x70);	// select register C
		inb(0x71);		// just throw away contents

}
