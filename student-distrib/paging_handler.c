#include "paging_handler.h"
#include "lib.h"
#include "types.h"
#include "syscalls.h"
 
#define FOURKB_PAGE_MASK  0xfffff000
#define FOURMB_PAGE_MASK 0xffC00000


/*  FILE:       paging_handler.c
 *  PROGRAMMER: Griffin A. Tucker
 *  DATE:		March 26 2017
 *  PURPOSE:	Holds functions for handling paging
 */

/*  FUNCTION:   enable_paging()
 *  PROGRAMMER: Griffin A. Tucker
 *  DATE:		March 26 2017
 *  PURPOSE:	Enables paging by initializing the directories and tables.
 */
void enable_paging()
{
	// An index used to step through the directories
	int ds_i;

	// An index used to step through the page directory
	int pd_i;

	// An index used to step through the page table
	int pt_i;

	// A temporary vector for holding calculated addresses
	int temp_vector;

	// Blank the directories
	// Each entry is set as not present and as R/W
	//PROCESSES = 8
	// PAGE_DIRECTORY_SIZE = 1024
	for(ds_i = 0; ds_i < PROCESSES; ds_i++)
	{
		for(pd_i = 0; pd_i < PAGE_DIRECTORY_SIZE; pd_i++)
		{
			page_directory_set[ds_i].entries[pd_i].vector = NP_BLANK_VECTOR;
		}
	}
	
	// Blank the first page table
	// Each entry is set as not present, and has a representitive address in memory
	// 4096
	//#define PAGE_TABLE_SIZE 1024

	for(pt_i = 0; pt_i < PAGE_TABLE_SIZE; pt_i++)
	{
		// calculate the address of the entry 
		temp_vector = 0x0;
		temp_vector = ((pt_i * PAGE_SIZE_4KB) & FOURKB_PAGE_MASK);
		temp_vector |= NP_BLANK_VECTOR;
		page_table.entries[pt_i].vector = temp_vector;
	}

	//Add the video memory entry to the first page table
    page_table.entries[VIDEO_IDX].vector |= P_BLANK_VECTOR;
    page_table.entries[VIDEO_IDX+1].vector |= P_BLANK_VECTOR;
    page_table.entries[VIDEO_IDX+2].vector |= P_BLANK_VECTOR;
    page_table.entries[VIDEO_IDX+3].vector |= P_BLANK_VECTOR;
    


	// // Add the first table to the first directory
	temp_vector = 0x0;
	temp_vector |= ((uint32_t)&page_table & FOURKB_PAGE_MASK);
	temp_vector |= P_BLANK_VECTOR;
	page_directory_set[0].entries[0].vector = temp_vector;

	// Add the kernal to the directory
	page_directory_set[0].entries[1].vector = KERNAL_START_VECTOR;

	// set the control registers to enable
	// one for initializing
	set_control_registers(1); 
}

void load_new_process(uint32_t process_num)
{

	// Error check for invalid process number. 
	if(process_num > 7 || process_num < 0)
	{
		printf("INVALID PROCESS NUMBER");
		return;
	}

	/* Declare vars */
	uint32_t temp_vector;                                                           // a temporary vector for holding clculated addresses

	// Load a 4_MB page into the directory for the correct program image location
	temp_vector = ((EIGHT_MB + (process_num * FOUR_MB)) & FOURMB_PAGE_MASK);		// Calculate the correct VA spot for the program image to map to
	//temp_vector |= IMAGE_INITIAL_ADDRESS;
	temp_vector |= IMAGE_BLANK_VECTOR;				
	//temp_vector |= IMAGE_ADDRESS_OFFSET;								            // Set the attribute bits
	page_directory_set[0].entries[IMAGE_ENTRY_IDX].vector = temp_vector;            // Set the vector

	// set control registers to enable
	// 0 for not initializing
    set_control_registers(0);

    // Return sucessful
    return;

    //////////////////////////////////////////////////////////////////////////////
    /*****************************************************************************
	EVERYTHING BELOW IS OLD AND BAD BUT KEEPING FOR THE SAKE OF MAYBE USEFUL >>>
    *****************************************************************************/
    //////////////////////////////////////////////////////////////////////////////

	// // /* Blank the new directory */
	//for(pd_i = 0; pd_i < PAGE_DIRECTORY_SIZE; pd_i++)
	//{
	//	page_directory_set[process_num].entries[pd_i].vector = NP_BLANK_VECTOR;
	//}

	// // // Blank the new page table 
	//for(pt_i = 0; pt_i < PAGE_TABLE_SIZE; pt_i++)
	//{
	//	temp_vector = 0x0;															// Clear the vector
	//	temp_vector = pt_i * PAGE_SIZE_4MB;											// Find the starting address
	//	temp_vector |= SET_SIZE_VECTOR; 											// Turn on the size bit
	//	temp_vector |= NP_BLANK_VECTOR;												// Set the Non-present bit
	//	new_page_table.entries[pt_i].vector = temp_vector;							// Set the vector
	//}

    /* Mark the first entry as present */
	//new_page_table.entries[0].vector |= 0x01;

	/* Initialize first directory address */
	//temp_vector =  0x0;																// Clear the vecotr
	//temp_vector |= (process_num+1) * PAGE_SIZE_4MB;									// Find the starting address
	//temp_vector |= SET_SIZE_VECTOR;													// Set the size bit
	//temp_vector |= P_BLANK_VECTOR;													// Set the present bit
	//page_directory_set[process_num].entries[0].vector = temp_vector;				// Set the vector

	/* Move the kernal to the new process */
	//page_directory_set[process_num].entries[1].vector = KERNAL_START_VECTOR;

	/* Initialize an entry for the program image */
	//temp_vector = 0x0;																// Clear the vector
	//temp_vector |= IMAGE_INITIAL_ADDRESS;											// Set the initial address to 128MB
	//temp_vector += (process_num * IMAGE_ADDRESS_OFFSET);							// Add an offset based on the process number
	//temp_vector |= IMAGE_BLANK_VECTOR;												// Or in the initial specifier bits
	//page_directory_set[process_num].entries[IMAGE_ENTRY_IDX].vector = temp_vector;  // Set the vector
}

void load_video_mem(uint32_t terminal_num)
{

	/* Declare vars */
	uint32_t temp_vector; 
	uint32_t PD_IDX;

	/* Calculate entry num in page directory */
	PD_IDX = V_M_ENTRY_IDX + terminal_num;

	// Init a page table for video memory
	temp_vector = VIDEO;
	temp_vector |= V_M_BLANK_VECTOR;
	v_m_page_table.entries[0].vector = temp_vector;

	// Clear temp vector
	temp_vector = 0x0;

	// Load a 4kb page for video memory
	temp_vector = ((uint32_t)&v_m_page_table & FOURKB_PAGE_MASK);
	temp_vector |= V_M_BLANK_VECTOR;			
	page_directory_set[0].entries[PD_IDX].vector = temp_vector;       

	// set control registers to enable
	// 0 for not initializing
    set_control_registers(0);

    // Return sucessful
    return;
}

// USED ONLY FOR TESTING PAGING. DO NOT USE OTHERWISE.
void init_new_vector(int pd_num, int entry_num, uint32_t vec)
{
	page_directory_set[pd_num].entries[entry_num].vector = vec;
}

void set_control_registers(int init)
{
	if(init == 1)
	{
		/* Set control registers */
		asm (
		"movl $page_directory_set, %%eax   ;" 
		"andl $0xFFFFFFE7, %%eax         ;"
		"movl %%eax, %%cr3               ;"
		"movl %%cr4, %%eax               ;"
		"orl $0x00000010, %%eax          ;"
		"movl %%eax, %%cr4               ;"
		"movl %%cr0, %%eax               ;"
		"orl $0x80000000, %%eax 	     ;"
		"movl %%eax, %%cr0                "
		: : : "eax", "cc" );

// 		asm (
// 		 "movl $page_directory_set, %%eax   ;" 
// 		 "andl $0xFFFFFFFF, %%eax         ;"
// 		 "movl %%eax, %%cr3               ;"
// 		 "movl %%cr4, %%eax               ;"
// 		 "orl $0x00000010, %%eax          ;"
// 	     "movl %%eax, %%cr4               ;"
// 		 "movl %%cr0, %%eax               ;"
// 		 "orl $0x80000000, %%eax 	     ;"
// 		 "movl %%eax, %%cr0                "
// 		 : : : "eax", "cc" );



// 		return;
	}
		/* Set control registers */
		

		asm (
		"movl $page_directory_set, %%eax   ;" 
		"andl $0xFFFFFFE7, %%eax         ;"
		"movl %%eax, %%cr3               ;"
		"movl %%cr4, %%eax               ;"
		"orl $0x00000090, %%eax          ;"
		"movl %%eax, %%cr4               ;"
		"movl %%cr0, %%eax               ;"
		"orl $0x80000000, %%eax 	     ;"
		"movl %%eax, %%cr0                "
		: : : "eax", "cc" );

	
}
