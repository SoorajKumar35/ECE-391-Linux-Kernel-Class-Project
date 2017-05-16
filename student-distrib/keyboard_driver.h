
#ifndef keyboard_driver_h 
#define keyboard_driver_h 

#include "types.h"

#define LINE_SIZE 128
#define CONTROL 0x3D4 
#define DATA 0x3D5 
#define KEYBOARD_DATA_PORT 0x60 
#define BACKSPACE 0x0E
#define CAPS_LOCK 0x3A
#define L_SHIFT 0x2A
#define L_SHIFT_P 0xAA
#define R_SHIFT 0x36
#define R_SHIFT_P 0xB6
#define TAB 0x0f
#define ENTER 0x1C
#define BACKSLASH 0x2B
#define CTRL 0x1D
#define RELEASE 0x80 
#define RELEASE_L 0xA6
#define RELEASE_1 0x82
#define RELEASE_2 0x83
#define RELEASE_3 0x84
#define MIN_HZ 0x02 //minumum frequency of RTC 
#define MAX_HZ 1024 //maximum frequency of RTC 
#define FILENAME_LENGTH 32 
#define VIDEO_MEM_SIZE 80*25
#define F3 0x3d

#ifndef ASM 
extern int cursor_x; 
extern int cursor_y; 
extern unsigned char input_buffer[LINE_SIZE]; //
extern unsigned char temp_buffer[LINE_SIZE]; 
extern int length_typed; //length typed by the user 
extern int old_length_typed; 
extern volatile int can_read; 
extern uint8_t terminal_one_buf[80*25];
extern uint8_t terminal_two_buf[80*25];
extern uint8_t terminal_three_buf[80*25];

extern int terminal_one_stored;
extern int terminal_two_stored;
extern int terminal_three_stored;


extern uint8_t current_terminal;

extern void move_cursor_to(int curs_x, int curs_y); 
void update_buffer_and_cursor(); //updates the buffer and adjusts the cursor (see idt.c for usage)
extern void flush(); //flushes the input buffer and clears the screen 
void clear_screen();
extern void scroll_if_needed(); 

#endif /* ASM */

#endif  
