#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#ifndef ASM

#include "lib.h"
#include "filesysobjs.h"
#include "keyboard_driver.h"
#include "i8259.h"

/* Number of file descriptors */
#define NUM_DESCRIPTORS 8

/* Indices for stdin and stdout in fd_array */
#define STDIN 0
#define STDOUT 1

#define SCREEN_BYTES 80*25*2

/* File types */
#define RTC 0
#define DIRECTORY 1
#define STANDARD 2

/* The ASCII value for a space */
#define ASCII_SPACE 0x20

/* Vector denoting full process occupation */
#define MAX_PROCESSES 0x3F
#define MAX_PROCESSES_DEC 6

/* Base entry point of the program image */
#define P_I_ENTRY_POINT 0x08048000

/* Base entry point of video memory */
#define V_M_ENTRY_POINT 0x08400000

/* Definitions of amounts of memory */
#define ONE_TWENTY_EIGHT_MB ((FOUR_MB/4) * 128)
#define ONE_THIRTY_TWO_MB ONE_TWENTY_EIGHT_MB + FOUR_MB
#define FOUR_MB 4194304
#define FOUR_KB 4096
#define EIGHT_MB 8388608
#define EIGHT_KB 8192

/* Terminal numbers */
#define TERMINAL_ONE 0

/* Maximum size for the arguments buffer 
   128 characters allowed for the input buffer 
  - 32 characters for maximum file name size
  -  1 character for one space */
#define MAX_ARG_BUF_SIZE 95

/* Misc. Definitions */
#define FILENAME_SIZE 32
#define EXECUTABLE_BYTES 4 
#define NO_PROCESSES 0 
#define HANDLED_LEADING_ZEROES 1 
#define IN_USE 1 

#define FIRST_TERM_IDX 0
#define SECOND_TERM_IDX 1
#define THIRD_TERM_IDX 2
  
typedef struct device_operations {
	uint32_t (*open) (const uint8_t* filename);
	uint32_t (*close) (uint32_t fd);
	uint32_t (*read) (uint32_t fd, const void* buf, uint32_t nbytes);
	uint32_t (*write) (uint32_t fd, const void* buf, uint32_t nbytes);
} device_operations;

typedef struct FILE_DATA { 
	// maybe add a flags variable
	device_operations *table; 
	int in_use; //not in use  
	uint32_t file_pos; 
	inode_t *info; 
} FILE_DATA;   

typedef struct process_control_block
{
	uint32_t process_num;
	uint32_t * parent_sp; //stack parent 
	uint32_t * parent_bp; //base pointer 
	struct process_control_block * parent_pcb; //parent 
	uint8_t    filename[FILENAME_SIZE]; // buffer to hold the name of the process
	uint8_t    args[MAX_ARG_BUF_SIZE]; // buffer to hold args for a program
	FILE_DATA  fd_array[NUM_DESCRIPTORS];
	int term_start;
} pcb_t;

typedef struct terminal 
{
	int been_here;
	int cursor_x;
	int cursor_y;
	uint8_t video_mem[2*80*25];
	uint8_t keyboard_buffer[LINE_SIZE];
	uint8_t * vid_mem;
	pcb_t * running_pcb;
	uint32_t * current_esp;
	uint32_t * current_ebp;
	uint32_t esp0;
}term;

extern term terms[3];


//extern FILE_DATA files[NUM_DESCRIPTORS]; 

extern pcb_t *running_pcb;

extern void switch_to_user(uint32_t correct_eip);

/* Vector which indicates which of the 8 processes is running */
extern uint8_t processes; 

extern uint8_t entry_point[4];    // holds the bits of a file which demarcate its entry point (bits 24-27)
extern uint32_t correct_eip; 

extern uint32_t eip_for_return;

extern int calc_processes();

int32_t halt (uint8_t status);
int32_t execute (const uint8_t * command);
int32_t read (uint32_t fd, const void* buf, uint32_t nbytes);
int32_t write (uint32_t fd, const void* buf, uint32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (uint32_t fd); 
int32_t getargs(uint8_t * buf, int32_t nbytes);
int32_t vidmap(uint8_t ** screen_start);
int32_t set_handler(int32_t signum, void * handler_address);
int32_t sigreturn(void);

int32_t closed_open(const uint8_t* filename);
int32_t closed_close(uint32_t fd);
int32_t closed_read(uint32_t fd, const void* buf, uint32_t nbytes);
int32_t closed_write(uint32_t fd, const void* buf, uint32_t nbytes);

extern void switch_to_other_term(int term);


#endif /* ASM */
 
#endif /* FILE_OPERATIONS_H */ 
