#include "lib.h"
#include "keyboard_driver.h"
#include "terminal_driver.h"
#include "i8259.h"
#include "syscalls.h"
#include "filesysobjs.h"
#include "paging_handler.h"
#define EIGHTY 80
#define TWENTY_FIVE 25
#define ONE_TWENTY_TWO 122
#define NINETY_SEVEN 97
int length_typed = 0; 
int cursor_x = 0; 
int cursor_y = 0;
unsigned char input_buffer[LINE_SIZE];
int ctrl_pressed = 0; 
int shift_flag = 0; 
int caps_flag = 0; 
volatile int can_read = 0; 
unsigned char temp_buffer[LINE_SIZE];

uint8_t terminal_one_buf[80*25];
uint8_t terminal_two_buf[80*25];
uint8_t terminal_three_buf[80*25];

int terminal_one_stored = 0;
int terminal_two_stored = 0;
int terminal_three_stored = 0;

int old_length_typed = 0; 
int rtc_interrupt_happened = 0; 
//the regular keyboard characters
int keyboard_char[100] = {
	'0', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
	'0', //backspace 
	'\t', //TAB
	'q','w','e','r','t','y','u','i','o','p', '[',']', 
	'0', //Enter
	'a', //Ctrl
	'a','s','d','f','g','h','j','k','l', ';', '\'', '`',
	'0', //Left Shift
	'\\', //backslash - giving me an issue
	'z', 'x','c','v','b','n','m',
	',','.','/','0','0','0',
	' ' //Space
};

//The shifter keyboard characters
int shift_keyboard_char[100] = {
	'0','0', '!', '@','#','$','%','^','&','*','(',')','_','+',
	'0','0',
	'0','0','0','0','0','0','0','0','0','0', '{','}',
	'0','0',
	'0','0','0','0','0','0','0','0','0', ':', '"', '~',
	'0','|',
	'0', '0','0','0','0','0','0',
	'<', '>', '?', 
};

/* FUNCTION:  move_cursor_to
   INPUTS:    curs_x -> the x coord, curs_y -> the y coord
   OUTPUTS:   
*/ 
void move_cursor_to(int curs_x, int curs_y) { 
	outb(0xE, CONTROL); //tell VGA we want to send the high byte 
	outb((curs_y * NUM_COLS + curs_x) >> 8, DATA); //send the high byte 
	outb(0xF, CONTROL); //tell VGA we want to send the low byte 
	outb(curs_y * NUM_COLS + curs_x, DATA); //send the low byte
}

/* FUNCTION:  clear_screen
   INPUTS:    
   OUTPUTS:   
*/ 
void clear_screen() { 
	cursor_x = 0;  
    cursor_y = 0; 
   	move_cursor_to(0, 0); //move the cursor to the upper left hand corner of the screen 
    ctrl_pressed = 0; //control button is not pressed 
    clear();  //clear the screen 
	
}
/* FUNCTION:  flush
   INPUTS:    
   OUTPUTS:   
*/ 
void flush() { 
	memset(input_buffer, 0, LINE_SIZE); //clear the input buffer  
    length_typed = 0; //reset size of buffer to zero 
}

/* FUNCTION:  scroll_if_needed
   INPUTS:    
   OUTPUTS:   
*/ 

void scroll_if_needed() { 
	int x_val; 
	int y_val; 
	if(cursor_y >= TWENTY_FIVE) { 
		x_val = 0; 
		y_val = 0; 

		//shift all rows up by one except for first one
		//screen is 25 * 80 (height * width) 
		for(y_val = 1; y_val < TWENTY_FIVE; y_val ++) { 
			for(x_val = 0; x_val < EIGHTY; x_val++) { 
				*(uint8_t *)(VIDEO + ((NUM_COLS * (y_val - 1) + x_val) << 1)) =  *(uint8_t *)(VIDEO + ((NUM_COLS * (y_val) + x_val) << 1)); 
	    		*(uint8_t *)(VIDEO + ((NUM_COLS * (y_val - 1) + x_val) << 1) + 1) = ATTRIB; 
			}
		}

		/* clear last row to give appearance of scrolling */
		for(x_val = 0; x_val < EIGHTY; x_val++) { 
			*(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS - 1) + x_val) << 1)) = ' '; 
		    *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS - 1) + x_val) << 1) + 1) = ATTRIB; 
		 }

		/* Set cursor_x back to beginning of line and decrement cursor_y to go back one row*/
		cursor_x = 0; 
		cursor_y--; 
		
		move_cursor_to(cursor_x, cursor_y); //adjust the position of the cursor on the screen 
	}
}

/* FUNCTION:  update_buffer_and_cursor
   INPUTS:    
   OUTPUTS:   
*/ 
void update_buffer_and_cursor() { 
	unsigned char scancode; //scancode (byte which will come from keyboard data port after interrupt has occurred)
    scancode = inb(KEYBOARD_DATA_PORT); //read a byte from the keyboard data port 
    static int term_count = 1;
    if(scancode & RELEASE) { 
    		if(shift_flag && (scancode == L_SHIFT_P || scancode == R_SHIFT_P)) { 
    				shift_flag = 0; //set the shift flag to zero to indicate shift is not pressed 
    		}
    		else if(scancode == RELEASE_L && ctrl_pressed == 1) { //check if control flag was set and that the key released was L 
    			flush(); //clear the input buffer 
    			clear_screen(); //clear the screen 
    		} 
    		else if(ctrl_pressed == 1 && scancode == RELEASE_1) 
    		{ 
    				term_count++;
    				if(current_terminal == FIRST_TERM_IDX)
    				{
    					asm ("jmp label1" );
    				}
    				// uint8_t * video_mem = (uint8_t *)VIDEO;
    				if(calc_processes() == MAX_PROCESSES_DEC || calc_processes() == 5)
    				{
    					//printf("Max number of processes have been reached");
    					asm("jmp label1");
    				}
    				//terms[current_terminal].been_here = 1;
    				switch_to_other_term(FIRST_TERM_IDX);

    		} 
    		else if(ctrl_pressed == 1 && scancode == RELEASE_2) 
    		{ 
    			term_count++;
    			// printf("I got here now plz lemme go to the other terminal my homie\n");
     			// uint8_t * video_mem = (uint8_t *)VIDEO;
    			if(current_terminal == SECOND_TERM_IDX)
    			{
    				asm("jmp label1");
    			}
    			if(calc_processes() == MAX_PROCESSES_DEC || calc_processes() == 5)
    			{
    				//printf("Max number of processes have been reached");
    				asm("jmp label1");

    			}
    			//terms[current_terminal].been_here = 1;
    			switch_to_other_term(SECOND_TERM_IDX);

			}
			else if(ctrl_pressed == 1 && scancode == RELEASE_3)
    		{

    			term_count++;
    			if(current_terminal == THIRD_TERM_IDX)
    			{
    				asm ("jmp label1" );
    			}
    		// uint8_t * video_mem = (uint8_t *)VIDEO;
    			if(calc_processes() == MAX_PROCESSES_DEC || calc_processes() == 5)
    			{
   		 			//printf("Max number of processes have been reached");
   	 				asm("jmp label1");
    			}
    			    				// asm ("jmp label1" );

    	//terms[current_terminal].been_here = 1;
    			switch_to_other_term(THIRD_TERM_IDX);
    			//printf("GOt after the third switch\n");
    		}
			ctrl_pressed = 0;
    	} 
    
	else if(scancode == F3 && ctrl_pressed == 1) 
	{

    	//NOTE:
    	//NOT accepting interrupts after the return
    	//Also have to change the value of the cursor
    	if(current_terminal == 3)
    	{
    		return;
    	}
    	if(MAX_PROCESSES == processes || (MAX_PROCESSES/2) == processes)
		{
			// printf("Max number of processes reached\n OR \n ");
			// printf("Five processes are currently running which means the\n second terminal cannot running another process\n");
			// return;
			char * warning_msg = "Max number of processes reached OR Five processes are currently running which means the\n second terminal cannot running another process\n ";
			int char_idx;
			uint8_t print_char;
			for(char_idx = 0; char_idx < strlen(warning_msg); char_idx++)
			{
				print_char = warning_msg[char_idx];
				if(print_char >= MIN_PRINTABLE) 
				{ 
					putc(print_char);
				}
				scroll_if_needed();
			}
			return;
		}
	}
    else if(ctrl_pressed == 0) {
    		if(cursor_x > NUM_COLS - 1) { 
    		 	cursor_x = 0;
    		 	cursor_y++; 
    		 }

    		if(scancode == CTRL) { //control key was pressed 
	    		ctrl_pressed = 1; //if control was pressed, we set a flag 
	    	} 

	    	else if(scancode == CAPS_LOCK) {  //if the button just pressed was CAPS_LOCK
	    		if(caps_flag == 1) { 
	    			caps_flag = 0; 
	    		} else { 
	    			 caps_flag = 1; 
	    		}

	    	}
	    	else if(scancode == L_SHIFT) {  //button pressed was LEFT SHIFT 
	    			shift_flag = 1; 
	    	} 

	    	else if(scancode == R_SHIFT) { 
	    		shift_flag = 1; 

	    	} else if(scancode == ENTER) { 
	    		//putc('\n'); //go to the next line
	    		/* update cursor values */ 
	    		//printf("ENTER PRESSED");
	    		cursor_x = 0;  
	    		cursor_y++; 
	    		memset(temp_buffer, 0, LINE_SIZE);
	    		memcpy(temp_buffer, input_buffer, LINE_SIZE); //copy command into temp_buffer
	    		old_length_typed = length_typed;
	    		flush(); //flush the input buffer (we are done with the current command)
	    		can_read = 1; //terminal_read can now read from the buffer 
	    		
	    	} else if(scancode == BACKSPACE && length_typed > 0) { //backspace occurred
	    		if(cursor_x == 0 && cursor_y != 0) { 
	    			cursor_x = EIGHTY - 1; //79 comes from 80 - 1 which is the width of the screen 
	    			cursor_y--;
	    			length_typed--;
	    			//printf("%d ", length_typed);
	    		} else if(cursor_x != 0) { 
	    			cursor_x--; 
	    			length_typed--;
	    			//printf("%d ", length_typed);
	    		} 

	    		*(uint8_t *)(VIDEO + ((NUM_COLS * cursor_y + cursor_x) << 1)) = ' '; 
	    		*(uint8_t *)(VIDEO + ((NUM_COLS * cursor_y + cursor_x) << 1) + 1) = ATTRIB; 

	    	} 

	    	else { 
	    		if(length_typed < LINE_SIZE && scancode != BACKSPACE) { //only allow input if the buffer isn't full 
	    			//printf(" (%d) ", scancode);
	    			unsigned char c = keyboard_char[scancode];
	    			if((shift_flag == 1 || caps_flag == 1) && c >= NINETY_SEVEN && c <= ONE_TWENTY_TWO) {  //only print printable characters 
			    	 	input_buffer[length_typed] = c - 32;
			    	 } else if((shift_flag == 1 || caps_flag == 1) && (c < NINETY_SEVEN || c > ONE_TWENTY_TWO)) { 
			    	 	input_buffer[length_typed] = shift_keyboard_char[scancode]; 
			    	 } else { 
			    	 	input_buffer[length_typed] = c; 
			    	 }
			    	*(uint8_t *)(VIDEO + ((NUM_COLS * cursor_y + cursor_x) << 1)) = input_buffer[length_typed];
			        *(uint8_t *)(VIDEO + ((NUM_COLS * cursor_y + cursor_x) << 1) + 1) = ATTRIB; 
			    	  length_typed++; 
			    	  cursor_x++; 
			    } 
	    	} 

	    	 //update the VGA cursor 
	    	 move_cursor_to(cursor_x, cursor_y); //update the cursor on the screen

		     scroll_if_needed(); //scroll if we need to 
		    
	    }
	    
    asm ("label1: ");
    send_eoi(0x01); //tell the PIC that we can accept more interrupts now

}

