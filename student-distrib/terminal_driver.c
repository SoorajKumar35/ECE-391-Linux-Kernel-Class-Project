#include "lib.h"
#include "idt.h"
#include "terminal_driver.h"
#include "keyboard_driver.h"
#include "syscalls.h"
/* FUNCTION:  initialize_terminal
   INPUTS:    
   OUTPUTS:   
*/ 
void initialize_terminal() { 
	cursor_x = 0; 
	cursor_y = 0;
	length_typed = 0;

	//volatile int can_read = 0; //initially we cannot read from the terminal since the user hasn't typed enter 
	//move to top left of screen 
	move_cursor_to(0, 0);
	clear(); 
}

/* FUNCTION:  terminal_open
   INPUTS:    filename -> pointer to file
   OUTPUTS:   
*/ 
uint32_t terminal_open(const uint8_t* filename) { 
	// printf("This is terminal_open\n");
	return 0; 
}

/* FUNCTION:  terminal_write
   INPUTS:    fd -> pointer to file, buf -> the buf to be written, nbytes->  the number of bytes to be written
   OUTPUTS:   
*/ 
uint32_t terminal_write(uint32_t fd, const void *buf, uint32_t nbytes) {

		// // printf("This is terminal_write\n");
		// if(fd != STDOUT) { 
		// 	return -1; 
		// }

		// if(cursor_x > NUMBER_COLUMNS - 1) { //if the cursor has gone past the end of a line 
		// 	cursor_x = 0; 
		// 	cursor_y++; 
		// }

		// int i; 
		

		// for(i = 0; i < nbytes; i++) { 
		// 	unsigned char c  = ((unsigned char *) buf)[i];
		// 		if(c >= MIN_PRINTABLE) { 
		// 			*(uint8_t *)(VIDEO + ((NUM_COLS * cursor_y + cursor_x) << 1)) = ((unsigned char *) buf)[i];
		// 		    *(uint8_t *)(VIDEO + ((NUM_COLS * cursor_y + cursor_x) << 1) + 1) = ATTRIB; 
		// 		    cursor_x++; 
		// 		    scroll_if_needed(); //scroll if we have written passed the bottom row of the screen 
		// 		    move_cursor_to(cursor_x, cursor_y); //move the cursor to the upper left hand corner of the screen 
		// 		}  
  
		// }
		//  //add a new line to the end of the write 
	 //    		/* update cursor values */ 
		// cursor_x = 0;  
		// cursor_y++; 
		// move_cursor_to(cursor_x, cursor_y); //move the cursor to the upper left hand corner of the screen 
		// scroll_if_needed(); 
		// flush(); //flush the input buffer (we are done with the current command)

		// return 0;

	// printf("This is terminal_write\n");
		if(fd != STDOUT) { 
			return -1; 
		}
		int i = 0; 
		uint8_t c = -1; 
		for(i = 0; i < nbytes; i++){
			c  = ((uint8_t *) buf)[i];
			if(c == '\0'){
				continue;
			}
			else if(c >= MIN_PRINTABLE || c == '\n') { 
				putc(c);
			} 
			scroll_if_needed();
		}
		
		flush(); //flush the input buffer (we are done with the current command)
		terms[current_terminal].cursor_x = cursor_x;
		terms[current_terminal].cursor_y = cursor_y;
		return 0; 
}

/* FUNCTION:  terminal_read
   INPUTS:    fd -> pointer to file, buf -> the buf to be written, nbytes->  the number of bytes to be written
   OUTPUTS:   
*/ 

uint32_t terminal_read(uint32_t fd, const void *buf, uint32_t nbytes) { 

	//printf("READ CALLED");
	if(fd != STDIN) { 
		return -1; 
	} 
	sti();
	//printf("This is terminal_read");
	memset((char *) buf, 0, nbytes); //clear the input buffer 
		
	while(can_read != 1); //spin until the user presses enter 
	//printf("in terminal_read");
	if(length_typed + 1 < nbytes) { 
		temp_buffer[old_length_typed] = '\n';
		memcpy((unsigned char *) buf, temp_buffer, old_length_typed + 1);
	} else { 
		temp_buffer[old_length_typed] = '\n';
		memcpy((unsigned char *) buf, temp_buffer, nbytes);
	}
	// printf("Current process filename %s\n", terms[current_terminal].running_pcb -> filename);
	// printf("Curretn process process num %d\n", terms[current_terminal].running_pcb->process_num);
	char * buf2 = (char * ) buf; 

	can_read = 0; 
	flush(); 

	return strlen(buf2); 
}
/* FUNCTION:  terminal_close
   INPUTS:    fd -> pointer to file, 
   OUTPUTS:   
*/ 
uint32_t terminal_close(uint32_t fd) { 
	// printf("This is terminal_close");
	return 0; 
}
