#include "lib.h"
#include "syscalls.h"
#include "terminal_driver.h"
#include "rtc.h"
#include "filesysobjs.h"
#include "paging_handler.h"
#include "x86_desc.h"

struct device_operations terminal_fops =  { 
	terminal_open, 
	terminal_close, 
	terminal_read, 
	terminal_write
}; 

struct device_operations rtc_fops =  { 
	(void *)rtc_open, 
	(void *)rtc_close, 
	(void *)rtc_read, 
	(void *)rtc_write
};

struct device_operations dir_fops =  { 
	(void *)dir_open, 
	(void *)dir_close, 
	(void *)dir_read, 
	(void *)dir_write
};

struct device_operations file_fops =  { 
	(void *)file_open, 
	(void *)file_close, 
	(void *)file_read, 
	(void *)file_write 
};


// Declare global variables
uint8_t processes; 												//the number of processes currently running
pcb_t * running_pcb; 											//the pcb of the current process running
uint8_t entry_point[EXECUTABLE_BYTES];

term terms[3];

// Declare global variables
uint8_t processes; 												//the number of processes currently running
pcb_t  *running_pcb; 											//the pcb of the current process running
uint32_t correct_eip;
uint32_t eip_for_return = 0;
uint32_t old_ebp = 0;
uint32_t old_esp = 0;
uint32_t ebp_val = 0;
uint32_t esp_store;
uint32_t ebp_store;

/* FUNCTION: 	HALT 
   DATE:		
   PURPOSE: halts the execution of the given process  
   INPUTS: 	current status of the program 
   OUTPUTS: returns 0 on success, -1 otherwise 
   RESULTS: 
*/

int32_t halt (uint8_t status)
{
	//printf("HALT CALLED");
	/* If no running PCB, fail */
	cli(); 
	if(terms[current_terminal].running_pcb == NULL) return -1;

	/* Declare variables */
	// uint32_t p_i; 												// Process number
	// uint32_t clear_process_vector; 								// Vector to be used for clearing a process from the processes vector
	// uint32_t clear_proc_bitmask;								// Bitmask used to help with clearing the process vector
	int i; 						   								// Generic iterator 
	
	// processes >>= 1;

	//Introduce Logic to change running pcb based on terminal
	uint32_t p_i = terms[current_terminal].running_pcb -> process_num;


	//use the running pcb of current terminal to change value of processes
	//This is an array that is used to clear a bit in proccesses depending on the process_num of the running_pcb process
	//we are halting
	unsigned char process_clear_bitmasks[6] = {0x3e, 0x3d, 0x3b,0x37,0x2f,0x1f};
	processes &= process_clear_bitmasks[p_i];
	//printf("Processes after halting %x\n",processes);
	//printf("HALT %#x\n", processes);
	/* Close all FD's currently open from this process */ 
	for(i = 0; i < PROCESSES; i++)
	{
		if (terms[current_terminal].running_pcb->fd_array[i].in_use == IN_USE)
		{			// If the file_descriptor is open, close it
			close(i);											// Close the fd
		}
	}

	/* Clear the args and filename buffers of the running pcb */
	strncpy((int8_t *)terms[current_terminal].running_pcb->args, "\0", MAX_ARG_BUF_SIZE);
	strncpy((int8_t *)terms[current_terminal].running_pcb->filename, "\0", FILENAME_SIZE);
	//printf("The value of running_pcb before restore = %d\n", running_pcb->process_num);
	
	/* Set the current_pcb to the pcb of the process 
	   being halted's parent process. If we are trying 
	   to halt the first shell, reload the first shell 
	   into paging. */

	   //BE CAREFUL OTHER TERMINALS COULD BE OPEN SO DON'T AXE A NEEDED GLOBAL VAR
	if(terms[current_terminal].running_pcb->process_num == 0 || terms[current_terminal].running_pcb->term_start == 1)
	{
		// old_ebp = (uint32_t) running_pcb->parent_bp;
		// old_esp = (uint32_t) running_pcb->parent_sp;
		// running_pcb = NULL;
		// load_new_process(0);
		// processes = 0;
		execute((uint8_t*)"shell");
	}
	/* Otherwise, restore the parent pcb as the current pcb */
	else
	{
		old_ebp = (uint32_t) terms[current_terminal].running_pcb->parent_bp;
		old_esp = (uint32_t) terms[current_terminal].running_pcb->parent_sp;
		terms[current_terminal].running_pcb = terms[current_terminal].running_pcb->parent_pcb;
		load_new_process(terms[current_terminal].running_pcb->process_num);
	}
	// printf("The value of running_pcb before restore = %d\n", running_pcb->process_num);
	
	/* Restore the base and stack pointers, and then jump 
	  //  to the parent process */
	//return 0; 
  	tss.esp0 =  EIGHT_MB - (EIGHT_KB * terms[current_terminal].running_pcb->process_num) - 4;
  	sti();
  	// tss.ss0 =  KERNEL_DS;
    asm("movl old_esp, %%esp ;"
    	"movl old_ebp, %%ebp "
    	: : :  "ebp", "esp");
    asm ("jmp label" );

    /* goto return_label; */
	return 0;

}

/* FUNCTION: executes a given file in the filesystem 
   DATE:		
   CODING: Griffin A. Tucker 	
   RESULTS: 	

   	returns 0 if execute happened, -1 otherwise 
*/

int32_t execute (const uint8_t* command)
{

	cli(); 

	/* Declare variables */
	dentry_t  d_entry;       									// the directory entry of the file
	uint8_t is_executable[EXECUTABLE_BYTES];   					// holds the bits of a file which determine executability
	int8_t file_name[FILENAME_SIZE]; 	  						// the name of the file to be executed
	uint8_t arg_buffer[MAX_ARG_BUF_SIZE]; 						// a temporary buffer to hold parsed arguments
	uint32_t cur_char = 0;	  									// iterative for looping through the file_name 
	uint32_t arg_cur_char = 0;									// iterative for looping through arg_buffer
	uint32_t file_cur_char = 0;									// iterative for looping through file buffer
	uint32_t * file_source;    									// the source address of the process
	uint32_t handled_lz = 0;  									// leading zeros handled 
	uint8_t p_i = 0;		  									// iterator for iterating across total processes
	uint8_t p_mask = 0x01;    									// a bitmask for finding an open process space 
	pcb_t * pcb;												// Pointer to the process control block for this function 
	int parsed_filename = 0;									// Have we parsed the filename? (0) -> no, (1) -> yes
	int x;
	pcb_t * parent_pcb = 0;
	/* Clear the file_name and arg_buffer buffers */
	strncpy((int8_t *)arg_buffer, "\0", MAX_ARG_BUF_SIZE);
	strncpy((int8_t *)file_name, "\0", FILENAME_SIZE);



	/* If command is invalid, fail */
	if(command == NULL) return -1;


	//INIT PROCESSES GLOBAL VAR IF NO PROCESS HAS BEEN STARTED AS OF YET
	/* If the process counter has not been initialized,
	   initialize it. Initialize the process counter 
	   to the first process being run keeps track of how 
	   many processes are running. */
	if(processes == NULL){
		processes = NO_PROCESSES;
	}

	/* Declare constant array of bits to determine if 
	   file is executable */
	const int8_t EXEC_BITS[EXECUTABLE_BYTES] 
		= {0x7F, 0x45, 0x4C, 0x46}; 							// Magic numbers to determine if file is executable

	
	/* THIS CODE IS FOR OBTAINING THE FILE NAME OF THE EXECUTABLE TO BE EXECUTED AS WELL AS 
	TO PARSE THE ARGS. ARGS ARE STORED IN THE PCB OF THE PROCESS */



	/* While we still have characters to read */
	while(cur_char < strlen((int8_t*)command))
	{
		/* Parse any leading spaces */
		if(command[cur_char] != ASCII_SPACE &&                  // If we are not at a space and handled_lz is not set as 1, set it.
		handled_lz == 0) 
		{
			handled_lz = HANDLED_LEADING_ZEROES;
		}

		/* Parse the filename */
		if(parsed_filename == 0 &&
		   handled_lz == HANDLED_LEADING_ZEROES)
		{
			/* If we have a non-space character to save */
			if(command[cur_char] != ASCII_SPACE &&	            // If we are not at a space and handled_lz is set as 1, save the char
			handled_lz == HANDLED_LEADING_ZEROES)
			{
				file_name[file_cur_char] = command[cur_char];	// Add a character from the command to file_name
				file_cur_char++;								// Increment the iterator for the file buffer
			}

			/* As soon as we see a space after handling 
			   leading zeros, we must be done with parsing 
			   the filename. */
			if(command[cur_char] == ASCII_SPACE &&				// If we are at a space and handled_lz is set as 1...
			handled_lz == HANDLED_LEADING_ZEROES)
			{ 
				parsed_filename = 1; 							// Set parsed_filename to 1 to indicate that we have finished parsing the filename.
				handled_lz = 0;									// Set handled_lz to 0 in order to handle leading zeros for the argument buffer.
            }
		}

		/* Parse the argument buffer */
		else
		{
			/* If we have handled the leading zeros before
			   the arguments */
			if(handled_lz == HANDLED_LEADING_ZEROES) 
			{
				arg_buffer[arg_cur_char] = command[cur_char];	// Add a character to the argument buffer
				arg_cur_char++;									// Increment the iterator for the arg buffer 
			}
		}

		// Move to the next character
		cur_char++;
	}

	/* Put endline character at end of file_name */
    file_name[cur_char] = '\0';

	/* Add a newline character to the arg_buffer */
	if(arg_buffer[0] != NULL) arg_buffer[arg_cur_char] = '\0';

	//printf("%s\n",file_name);
	//printf("%s\n",arg_buffer);


	/*THIS CODE IS FOR CHECKING WHETHER THE FILENAME IS A EVEN IN THE FILE SYSTEM
	AND WHETHER THAT FILE IS AN EXECUTABLE */

	// Check if the file specified is executable
	if((x = read_dentry_by_name((uint8_t *) file_name,
	&d_entry)) == -1) 
	{ 
			return -1;   
	}      

	// Read the 4 bits which distinguish the file as executable
	if(read_data(d_entry.inode_num, 0, is_executable,
	(int32_t) EXECUTABLE_BYTES) == -1) 
	{ 
		return -1;
	} 
	
	// Determine if the file is executable
	if(strncmp((int8_t *) is_executable, EXEC_BITS,
	(int32_t) EXECUTABLE_BYTES) != 0)  
	{   
		return -1;		
	}

	/*THE FOLLOWING CODE IS CHECKING WHETHER WE REACHED THE MAXIMUM PROCESSES AND IF NOT, 
	TO DETERMINE THE PROCESS ID FOR THE PROCESS CURRENTLY BEING EXECUTED*/

	//Check if we cannot run another process
	if(MAX_PROCESSES == processes)
	{
		printf("Max number of processes reached\n");
		return -10;
	}

	// Find an open process space
	//This is ok since we might need to axe processes before 6 and so to assign the proper 
	//process num to the pcb we need the correct place the prev process was axxed
	for(p_i = 0; p_i < MAX_PROCESSES_DEC; p_i++)
	{
		if((processes & p_mask) == 0)
		{
			processes |= p_mask;
			break;
		}
		p_mask <<= 1;
	}

	// Set up paging
	//printf("Current Process: %d", p_i);
	load_new_process(p_i); 

	// Read the entry point of the file
	// Grab bytes 24 - 27 to get the entry point to file
	/*
	if(read_data(d_entry.inode_num, 6 * EXECUTABLE_BYTES, 
	entry_point, (uint32_t) EXECUTABLE_BYTES) == -1) 
	{ 
		return -1; 
	} 
	*/

	/*WE ARE GETTING THE ENTRY POINT OF THE EXECUTABLE IN THIS CODE - USED FOR SWITCHING TO USER SPACE*/

	if(read_data(d_entry.inode_num, 6 * EXECUTABLE_BYTES, entry_point, (uint32_t) EXECUTABLE_BYTES) == -1) { 
			//printf("%s", "read data failed");
			return -1; 
		} 

	/*THE FOLLOWING CODE IS FOR FIRST EXTRACTING ALL THE DATA ASSOCIATED WITH AN EXECUTABLE AND THEN COPYING THAT DATA
	INTO MEMORY. GOTCHA FAM U SO SMART LOL
	*/

	//printf("After first read data");
	// Load the file into memory
	file_source = (uint32_t*)(start_fs + FS_BLOCK_SIZE +
	              (d_entry.inode_num * FS_BLOCK_SIZE));
	// uint8_t buf[*file_source];
	read_data(d_entry.inode_num, 0, (uint8_t*)P_I_ENTRY_POINT, *file_source);
	//memcpy((uint32_t*)P_I_ENTRY_POINT, buf, *file_source);

	/*HERE WE INIT THE PCB FOR ABOUT TO BE EXECUTED PROCESS*/


	// Instantiate new PCB for this process
	pcb = (pcb_t*)(EIGHT_MB - (EIGHT_KB * (p_i+1))); 				

	if(terms[current_terminal].been_here == 1)
	{
		parent_pcb = terms[current_terminal].running_pcb;
	}
	else
	{
		parent_pcb = 0;
	}
	// Update the global reference to the current pcb
	terms[current_terminal].running_pcb = pcb;
	if(terms[current_terminal].been_here != 1)
	{
		terms[current_terminal].running_pcb -> term_start = 1;
	}
	terms[current_terminal].been_here = 1;
	// Set process number in PCB
	terms[current_terminal].running_pcb->process_num = p_i;
	//printf("current_terminal %d\n", current_terminal);

	/* Clear the args and filename buffers of the running pcb */
	strncpy((int8_t *)terms[current_terminal].running_pcb->args, "\0", MAX_ARG_BUF_SIZE);
	strncpy((int8_t *)terms[current_terminal].running_pcb->filename, "\0", FILENAME_SIZE);

	/* Set filename in pcb */
	strncpy((int8_t*)terms[current_terminal].running_pcb->filename,
	        (int8_t*)file_name, 
	        FILENAME_SIZE);

	// If we are the first process being execute, 
	// no parent to access
	if(p_i == 0)
	{
	
		terms[current_terminal].running_pcb->parent_sp = 0x0;
		terms[current_terminal].running_pcb->parent_bp = 0x0;
		terms[current_terminal].running_pcb->parent_pcb = 0x0;

	}
	// If we are a new process, find parent pointers
	else
	{
	
		// Declare temp vars
		uint32_t  temp_esp = 0;
		uint32_t  temp_ebp = 0;

		// Get relevant pointers
		asm ("movl %%esp, %0":"=g"(temp_esp));
		asm ("movl %%ebp, %0":"=g"(temp_ebp));
	   
		// // parent_pcb = (pcb_t*)(temp_ebp + EIGHT_KB);
		// parent_pcb = (pcb_t *)
		// 				(EIGHT_MB - (EIGHT_KB * (p_i)));
//
	  //parent_pcb->process_num = running_pcb->process_num - 1; 
	 //   //parent_pcb->filename = (uint8_t *) "shell";
	 // printf("Parent process id: %d\n", parent_pcb->process_num);
	 // printf("parent process filename :%s\n", parent_pcb->filename);
		// Store relevant pointers in PCB
		terms[current_terminal].running_pcb->parent_sp = (uint32_t*)temp_esp;
		terms[current_terminal].running_pcb->parent_bp = (uint32_t*)temp_ebp;
		terms[current_terminal].running_pcb->parent_pcb = parent_pcb; 
	}

	/* Store the args of command in the running pcb */
	if(strlen((int8_t*)arg_buffer) != 0)								    
	{															
		if(strlen((int8_t*)arg_buffer) > MAX_ARG_BUF_SIZE) 		// If the buffer we are given is too large,
		{														// store the maximum number of bytes we
			strncpy((int8_t*)terms[current_terminal].running_pcb->args,  				// can store within the running pcb
				    (int8_t*)arg_buffer, 
				     MAX_ARG_BUF_SIZE);
		}
		else													// Otherwise, store the full arg buffer
		{														// within the running pcb
			strncpy((int8_t*)terms[current_terminal].running_pcb->args, 
				    (int8_t*)arg_buffer, 
				     strlen((int8_t*)arg_buffer));
		}
	}


	/*HERE WE MODIFY THE TSS FOR SWITCHING TO USER SPACE AND ALSO HAVE A LABEL FOR WHEN WE WANT TO RETURN FROM HALT
	ALSO WE SWITCH TO USER THROUGH THE SWITCH_TO_FUNCTION*/
	// Open FDs 
	open((uint8_t *) "stdin"); 
	open((uint8_t *) "stdout");

    uint32_t * eip_val = (uint32_t *) &entry_point;
    correct_eip = *eip_val;
    tss.esp0 = EIGHT_MB - (EIGHT_KB * p_i) - 4;
    tss.ss0 =  KERNEL_DS;

    switch_to_user(correct_eip);

	asm ("label:  ;"); 
	sti(); 

  	return 0;

}
 
/* FUNCTION: open	
   INPUTS:		filename - unsigned int rep. file
   OUTPUTS: 	unsigned integer signif ying success or failure
*/ 
/* opens the device with the corresponding file name */ 
int32_t open(const uint8_t* filename) { 

	/* Declare local variables */
	const char * STDIN_ID = "stdin";   // Identifing name for stdin file descriptor
	int stdin_length = 5; 
	int stdout_length = 6; 
    const char * STDOUT_ID = "stdout"; // Identifying name for stdout file descriptor 

	/* The file descriptor which has been opened. Returned by the function. */
	uint32_t fd = -1; 

	// If we are opening stdin...
	if(strncmp((int8_t *) filename, STDIN_ID, stdin_length) == 0)
	{

			//printf("1\n");

	//	printf("We are in the comparison for stdin\n");
		// If stdin is already open, fail
		if(terms[current_terminal].running_pcb->fd_array[STDIN].in_use == 1) 
		{ 
		//	printf("pcb was in use in open stdin. failing. \n");
			return -1; 
		}


		// printf("got here");
		// Otherwise, init stdin and open
		terms[current_terminal].running_pcb->fd_array[STDIN].table = &terminal_fops; 
		terms[current_terminal].running_pcb->fd_array[STDIN].in_use = 1; 
		terms[current_terminal].running_pcb->fd_array[STDIN].info = NULL; 
		terms[current_terminal].running_pcb->fd_array[STDIN].file_pos = 0;
		fd = terms[current_terminal].running_pcb->fd_array[STDIN].table->open(filename); 
 	} 

 	// If we are opening stdout...
 	//int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n);
 	else if(strncmp((int8_t *) filename, STDOUT_ID, stdout_length) == 0)
 	{
 		//printf("2\n");

		// printf("We are in the comparison for stdout\n");

 		// If stdout is already open, fail
 		if(terms[current_terminal].running_pcb->fd_array[STDOUT].in_use == 1) 
 		{ 

 			// printf("pcb was in use in open stdout. failing. \n");
 			return -1;
 		}

 		// Otherwise, init stdout and open
 		terms[current_terminal].running_pcb->fd_array[STDOUT].table = &terminal_fops; 
		terms[current_terminal].running_pcb->fd_array[STDOUT].in_use = 1; 
		terms[current_terminal].running_pcb->fd_array[STDOUT].info = NULL; 
		terms[current_terminal].running_pcb->fd_array[STDOUT].file_pos = 0;
		fd = terms[current_terminal].running_pcb->fd_array[STDOUT].table->open(filename);
		
 	} 

 	// If we are opening some fd [2,8]...
 	else {


	 dentry_t file_dentry;

	 if(read_dentry_by_name((uint8_t *) filename, &file_dentry) != 0){
	 	//printf("%s\n", filename);
	 	return -1;
	 }

 		// Loop through the fd_array until we find an open file descriptor.
 		// Note: if we find no open file descriptor, fail and return -1.
 		// printf("we didn't open either stdout or stdin");
 		 int index;  
 		 for(index = 2; index < 8; index++) 
 		 {
 		 	if(terms[current_terminal].running_pcb->fd_array[index].in_use == 0) 
 		 	{ 


 		 		// If the file is of type RTC, init the fd and open
 		 		if(file_dentry.file_type == RTC) 
 		 		{ 
 		 			
 		 			terms[current_terminal].running_pcb->fd_array[index].table = &rtc_fops; 
			 		terms[current_terminal].running_pcb->fd_array[index].in_use = 1; 
			 		terms[current_terminal].running_pcb->fd_array[index].info = (inode_t*)file_dentry.inode_num; 
			 		terms[current_terminal].running_pcb->fd_array[index].file_pos = 0;
			 		terms[current_terminal].running_pcb->fd_array[index].table->open(filename); 
			 		fd = index; 
			 		break; 
 		 		}

 		 		// If the file is of type directory, init the fd and open
 		 		if(file_dentry.file_type == DIRECTORY)
 		 		{

 		 			terms[current_terminal].running_pcb->fd_array[index].table = &dir_fops;
 		 			terms[current_terminal].running_pcb->fd_array[index].in_use = 1;
 		 			terms[current_terminal].running_pcb->fd_array[index].info = (inode_t*)file_dentry.inode_num;
 		 			terms[current_terminal].running_pcb->fd_array[index].file_pos = 0; 
 		 			terms[current_terminal].running_pcb->fd_array[index].table->open(filename);
 		 			fd = index;
 		 			break;
 		 		}

 		 		// If the file is of standard type, init the fd and open
 		 		if(file_dentry.file_type == STANDARD)
 		 		{
 		 			terms[current_terminal].running_pcb->fd_array[index].table = &file_fops;
 		 			terms[current_terminal].running_pcb->fd_array[index].in_use = 1;
 		 			terms[current_terminal].running_pcb->fd_array[index].info = (inode_t*)file_dentry.inode_num;
 		 			terms[current_terminal].running_pcb->fd_array[index].file_pos = 0;
 		 			terms[current_terminal].running_pcb->fd_array[index].table->open(filename);
 		 			fd = index;
 		 			break;
 		 		}
 		 	}
 		 }
 	}

 	return fd; 
}

/* FUNCTION: close	
   INPUTS:		fd -> file, buf -> what we are writing nbytes -> how much to write
   OUTPUTS: 	unsigned integer signifying success or failure
*/ 
int32_t write(uint32_t fd, const void* buf, uint32_t nbytes) { 

	//printf("This is generic write\n");
	//printf("fd, %d\n", fd);
	// Checking in bounds process reference
	if (fd < 0 || fd > PROCESSES - 1)
		return -1;

	// Checking if buffer is valid
	if (buf == NULL)
		return -1;

	// Checking is we have a running process to write to
	if (terms[current_terminal].running_pcb->fd_array[fd].in_use == 0)
		return -1;

    return terms[current_terminal].running_pcb->fd_array[fd].table->write(fd, buf, nbytes); 

}

/* FUNCTION: read	
   INPUTS:		fd -> file, buf -> what we are writing nbytes -> how much to write
   OUTPUTS: 	unsigned integer signifying success or failure
*/ 
int32_t read(uint32_t fd, const void* buf, uint32_t nbytes) { 

	//printf("P, %d\n", PROCESSES);
//	printf("read correctly");

	//Checking in bounds process reference
	if (fd < 0 || fd > PROCESSES - 1)
		return -1;

	//printf("This is generic read1\n");

	// Checking if buffer is valid
	if (buf == NULL)
		return -1;


	// Checking if we have a running process to read from
	if (terms[current_terminal].running_pcb->fd_array[fd].in_use == 0)
		return -1;

	//Read from process
	//printf("hope to make it this far\n");
	//uint32_t retval = running_pcb->fd_array[fd].table->read(fd, buf, nbytes);
	
	return terms[current_terminal].running_pcb->fd_array[fd].table->read(fd, buf, nbytes);
}

/* FUNCTION: 	close
   INPUTS:		fd -> file
   OUTPUTS: 	unsigned integer signifying success or failure
*/ 
int32_t close(uint32_t fd) { 
	//printf("This is gerneric close\n");	


	// Checking in bounds process reference. We cannot close stdout and stdin
	if (fd <= 1 || fd > PROCESSES - 1)
		return -1;

	// Check if process is running; if the process is running, 
	// set it to not in use, otherwise return -1 as we cannot
	// close a closed process
	if (terms[current_terminal].running_pcb->fd_array[fd].in_use == 0) 
		return -1;
	else terms[current_terminal].running_pcb->fd_array[fd].in_use = 0;

	// If closing fails, fail, otherwise, return sucessful
	if (0 != terms[current_terminal].running_pcb->fd_array[fd].table->close(fd))
		return -1;
	
	//printf("FD is %d", fd);
	return 0;
} 

/* FUNCTION: 	getargs
   PURPOSE:     The getargs syscall reads the programs's command line arguments into the
   				pcb of the current process. If the arguments and a terminal NULL (0-byte) 
   				do not fit in the buffer, simply return -1. 
   INPUTS:		buf -> user level buffer to read arguments from
   				nbytes -> number of bytes to read
   OUTPUTS: 	unsigned integer signifying success or failure
*/ 
int32_t getargs(uint8_t * buf, int32_t nbytes)
{ 
	/* Error checking */
	//printf("get args");
	if(buf == NULL) return -1; 											// If buf is a null pointer
	if(terms[current_terminal].running_pcb->args[0] == NULL) return -1;							// If we are given nothing to parse, fail
	if(nbytes <= 0) return -1;	        								// If we try to copy an invalid number of bytes, fail.
	if(strlen((int8_t *)terms[current_terminal].running_pcb->args) < nbytes);

	/* Declare local vars */
	int i = 0;															// Generic iterator

	/* Insure that the destination buffer is clear */
	memcpy(buf, "\0", strlen((int8_t *)buf));
	
	// Copy the args into the user space buffer
	while(terms[current_terminal].running_pcb->args[i] != '\0' && i < strlen((int8_t *)terms[current_terminal].running_pcb->args)) {
		buf[i] = terms[current_terminal].running_pcb->args[i];
		i++;
	}
	while(i < MAX_ARG_BUF_SIZE) {
		buf[i] = '\0';
		i++;
	}

	// Return successful
	return 0;
}

/* FUNCTION: 	vidmap
   PURPOSE:     The vidmap function maps the text-mode video memory into user
                space at a pre-set virtual address. Although the address returned
                is always the same, it should be written into the memory location
                provided by the caller, which is checked for validity. If the
                locatio nis invalid, return -1. 
   INPUTS:		screen_start -> the pointer to save to
   OUTPUTS: 	unsigned integer signifying success or failure
*/ 
int32_t vidmap(uint8_t ** screen_start)
{ 
	// Check if screen_start is a valid address to write to, i.e., 
	// screen_start is within the memory allotted to the program
	// image. This checks if screen_start is a valid user variable.
	if((uint32_t)screen_start >= ONE_TWENTY_EIGHT_MB &&
	   (uint32_t)screen_start < ONE_THIRTY_TWO_MB)
	{

		// Set up a 4Kb page space for the video memory
		load_video_mem(TERMINAL_ONE);

		// Store the base address of video memory space in screen_start
		*screen_start = (uint8_t *)ONE_THIRTY_TWO_MB;

		// Return Sucessful
		return ONE_THIRTY_TWO_MB;
	}

	// Return Failure
	return -1;
}

int32_t set_handler(int32_t signum, void * handler_address){ return -1;}
int32_t sigreturn(void){ return -1;}

/*
	These functions are to be placed in the f_ops table for a closed
	file descriptor. If the fd is accessed and a function is 
	attempted to be carried out through fluke, -1 is returned.
*/
int32_t closed_open(const uint8_t* filename){return -1;}
int32_t closed_close(uint32_t fd){return -1;}
int32_t closed_read(uint32_t fd, const void* buf, uint32_t nbytes){return -1;}
int32_t closed_write(uint32_t fd, const void* buf, uint32_t nbytes){return -1;}

int calc_processes()
{
	int i = 0;
	unsigned char processes_bit_mask = 0x01;
	uint32_t process_count;
	for(i = 0; i < 6; i++)
	{
		//Casting as unsigned char as I don't ever want the if statement to think that the result of the AND is neg, 
		//destroying its functionality
		if((uint8_t)(processes & processes_bit_mask))
		{
			process_count++;
		}
	}
	return process_count;
}

void switch_to_other_term(int term)
{
	if(terms[term].been_here != 1)
	{
		/*This code is for saving the current state of terminal we are switching FROM*/
		//Copying Video Mem into a buffer
		memcpy(terms[current_terminal].vid_mem, (uint8_t *)VIDEO, SCREEN_BYTES);
		//Copy keyboard buffer? MUST CONFIRM
		memcpy((uint8_t *)terms[current_terminal].keyboard_buffer, (uint8_t *)input_buffer, LINE_SIZE);
		//Save the state of the cursors for this terminal
		terms[current_terminal].cursor_x = cursor_x;
		terms[current_terminal].cursor_y = cursor_y;
		//Save the ebp and esp of the kernel stack of the process that is currently executing in the this terminal
		asm("movl %%esp, %%eax ;"
			"movl %%eax, esp_store ;"
    		"movl %%ebp, %%eax; "
    		"movl %%eax, ebp_store ;"
    	: : :  "ebp", "esp");
    	terms[current_terminal].current_esp = (uint32_t *)esp_store;
    	terms[current_terminal].current_ebp = (uint32_t *)ebp_store;

    	terms[current_terminal].esp0 = tss.esp0;


    	/*This code is for clearing the screen, sending an eoi to renable interrupts, changing the current terminal value 
    	to the value just passed in and then executing shell*/
		clear_screen();
		send_eoi(0x01);
		current_terminal = term;
		execute((uint8_t *)"shell");
	}
	else
	{
		/*This code is for saving the current state of terminal we are switching FROM*/
		//Copying Video Mem into a buffer
		memcpy((uint8_t*)terms[current_terminal].vid_mem, (uint8_t *)VIDEO, SCREEN_BYTES);
		//Copy keyboard buffer? MUST CONFIRM
		memcpy((uint8_t *)terms[current_terminal].keyboard_buffer, (uint8_t *)input_buffer, LINE_SIZE);
		//Save the state of the cursors for this terminal		
		terms[current_terminal].cursor_x = cursor_x;
		terms[current_terminal].cursor_y = cursor_y;
		//Save the ebp and esp of the kernel stack of the process that is currently executing in the this terminal
		asm("movl %%esp, %%eax ;"
			"movl %%eax, esp_store ;"
    		"movl %%ebp, %%eax; "
    		"movl %%eax, ebp_store ;"
    	: : :  "ebp", "esp");
    	terms[current_terminal].current_esp = (uint32_t *)esp_store;
    	terms[current_terminal].current_ebp = (uint32_t *)ebp_store;
    	    	terms[current_terminal].esp0 = tss.esp0;
    	/*This code is for renabling interrupts, chaning current_terminal appropriately*/
    	send_eoi(0x01);

		current_terminal = term;
		// printf("AFTER term = %d, current_terminal %d\n",term,current_terminal );
		//Restoring the state of video memory
		memcpy((uint8_t *)VIDEO, terms[current_terminal].vid_mem, SCREEN_BYTES);
		//Resotring the state of the keyboard buffer - MUST CONFIRM
		memcpy((uint8_t *)input_buffer, (uint8_t*)terms[current_terminal].keyboard_buffer,LINE_SIZE);
		//Restoring the state of the cursors
		cursor_x = terms[current_terminal].cursor_x;
		cursor_y = terms[current_terminal].cursor_y;
		move_cursor_to(cursor_x, cursor_y);
		//Restoring the esp and ebp of the kernel stack of the process that was executing in this terminal
		esp_store = (uint32_t)terms[current_terminal].current_esp;
		ebp_store = (uint32_t)terms[current_terminal].current_ebp;
    	load_new_process(terms[current_terminal].running_pcb->process_num);

		tss.esp0 =     	terms[current_terminal].esp0;
		asm("movl esp_store, %%esp ;"
    		"movl ebp_store, %%ebp ;"
    	: : :  "ebp", "esp");	

    	//Remapping virutual mem for the process currently running in this terminal
	}
}


