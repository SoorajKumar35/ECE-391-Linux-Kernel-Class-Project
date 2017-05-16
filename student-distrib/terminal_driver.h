
#ifndef TERMINAL_DRIVER_H
#define TERMINAL_DRIVER_H
#define LINE_SIZE 128 
#define STDOUT 1
#define STDIN 0 
#define NUMBER_COLUMNS 80
#define MIN_PRINTABLE 32 

#ifndef ASM

void initialize_terminal(); 
uint32_t terminal_open(const uint8_t* filename);
uint32_t terminal_close(uint32_t fd);
uint32_t terminal_write(uint32_t fd, const void * buf, uint32_t nbytes);
uint32_t terminal_read(uint32_t fd, const void * buf, uint32_t nbytes);


#endif /* ASM */


#endif /* TERMINAL_DRIVER_H  */ 
