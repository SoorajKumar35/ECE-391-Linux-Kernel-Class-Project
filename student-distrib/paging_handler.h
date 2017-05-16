/*  FILE:       paging_handler.h
 *  PROGRAMMER: Griffin A. Tucker
 *  DATE:		March 26 2017
 *  PURPOSE:	Serves as the header file for paging_handler.c
 */

#ifndef PAGING_HANDLER_H
#define PAGING_HANDLER_H

#include "types.h"

/* Number of processes */
#define PROCESSES 8

/* The number of spaces to shift to or an address into a vector */
#define ADDRESS_SHIFT 12
#define ADDRESS_SHIFT_4MB 8

/* Size of a page directory */
#define PAGE_DIRECTORY_SIZE 1024

/* Size of a page table */
#define PAGE_TABLE_SIZE 1024

/* The size of a standard 4KB page */
#define PAGE_SIZE_4KB 4096

/* The size of a standard 4MB page */
#define PAGE_SIZE_4MB 4194304  

/* The index of the program image entry point in the page directory. */
/* _128MB / 4_MB = 32 -> 0x20 */
#define IMAGE_ENTRY_IDX 0x20

/* The index of the virtual memor y entry point in the page directory. */
#define V_M_ENTRY_IDX 0x21

/* A blank vector for initialization. The present bit is not set. The R/W bit is set.*/
#define NP_BLANK_VECTOR 0x00000002

/* A blank vector for initialization. The present bit is set. The R/W bit is set. */
#define P_BLANK_VECTOR 0x00000003

/* The base vector for video memory */
#define V_M_BLANK_VECTOR 0x00000007

/* Used to set the size bit of a vector. */
#define SET_SIZE_VECTOR 0x00000080

/* A blank vector for the program image */
/* A vector of the starting address of the program image */
/* The offset amount for each process from the program image */
#define IMAGE_BLANK_VECTOR 0x00000087
#define IMAGE_INITIAL_ADDRESS 0x08000000
#define IMAGE_ADDRESS_OFFSET 0x00048000

/* Definitions of amounts of memory */
#define FOUR_MB 4194304
#define FOUR_KB 4096
#define EIGHT_MB 8388608
#define EIGHT_KB 8192

#define FAKE_VID_ONE 0xB9000
#define FAKE_VID_TWO 0xBA000
#define FAKE_VID_THREE 0xBB000


/* The starting vector for the kernal
   The kernal begins at 4MB of memory.
   R/W is set, present is set, P-S is set. A is set. See diagrams below.
*/
#define KERNAL_START_VECTOR 0x00400083

	/* 4KB Page Directory Entry
	#
	# The layout is (from Intel IA-32 reference manual):
	#  31                    12  11    9   8                               0
	# |----------------------------------------------------------------------|
	# |                        |         | G | P |   |   | C | W | U | R |   |
	# | Page Table Base Adress |  Avail  | - | - | R | A | - | - | - | - | P |
	# |                        |         | P | S |   |   | D | T | S | W |   |
	# |----------------------------------------------------------------------| */

typedef union PDE_t
{
	uint32_t val;
	struct 
	{
		uint32_t vector: 32;
	} __attribute__((packed));	
} PDE_t;

	/* 4KB Page Table Entry
	#
	# The layout is (from Intel IA-32 reference manual):
	#  31                    12  11    9   8                               0
	# |----------------------------------------------------------------------|
	# |                        |         | G | P |   |   | C | W | U | R |   |
	# |    Page Base Adress    |  Avail  | - | T | D | A | - | - | - | - | P |
	# |                        |         | P | A |   |   | D | T | S | W |   |
	# |----------------------------------------------------------------------| */

	/* 4MB Page Table Entry
	#
	# The layout is (from Intel IA-32 reference manual):
	#  31                13  12  11    9   8                               0
	# |----------------------------------------------------------------------|
	# |         |          | P |         | G | P |   |   | C | W | U | R |   |
	# | PB Addr | Reserved | T |  Avail  | - | - | D | A | - | - | - | - | P |
	# |         |          | A |         | P | S |   |   | D | T | S | W |   |
	# |----------------------------------------------------------------------| */

typedef union PTE_t
{
	uint32_t val;
	struct 
	{
		uint32_t vector: 32;
	} __attribute__((packed));	
} PTE_t;

/* Page directory struct */

typedef struct page_directory_t
{
	PDE_t entries[PAGE_DIRECTORY_SIZE];
} page_directory_t;

/* Page Table struct */
typedef struct page_table_t
{
	PTE_t entries[PAGE_TABLE_SIZE];
} page_table_t;

/* Eight page directories, one for each process */
page_directory_t page_directory_set[PROCESSES] __attribute__((aligned (PAGE_SIZE_4KB * PROCESSES)));

/* One page table */
page_table_t page_table __attribute__((aligned(PAGE_SIZE_4KB)));

/* One page table for video memory */
page_table_t v_m_page_table __attribute__((aligned(PAGE_SIZE_4KB)));

/* Function Definitions */
void enable_paging();
void set_control_registers(int init);
void init_new_vector(int pd_num, int entry_num, uint32_t vec);
void load_new_process(uint32_t process_num);
void load_video_mem(uint32_t terminal_num);

#endif /* PAGING_HANDLER_H */
