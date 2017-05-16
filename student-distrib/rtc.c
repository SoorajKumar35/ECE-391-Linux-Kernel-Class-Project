#include "rtc.h"
#include "lib.h"

#define rate_for_1024 6
#define rate_for_512 7
#define rate_for_256 8 
#define rate_for_128 9 
#define rate_for_64 10 
#define rate_for_32 11
#define rate_for_16 12 
#define rate_for_8 13 
#define rate_for_4 14 
#define rate_for_2 15 

#define TWO_HZ 2 
#define FOUR_HZ 4 
#define EIGHT_HZ 8 
#define SIXTEEN_HZ 16 
#define THIRTY_TWO_HZ 32 
#define SIXTY_FOUR_HZ 64 
#define ONE_TWENTY_EIGHT_HZ 128
#define TWO_FIFTY_SIX_HZ 256  
#define FIVE_TWELVE_HZ 512 
#define ONE_THOUSAND_TWENTY_FOUR_HZ 1024
#define BASE_RATE 15 
#define MASK_ONE 0x40
#define MASK_TWO 0x0F
#define MASK_THREE 0xF0


/*
	DESCRIPTION: This function initializes the rtc

*/
uint32_t rtc_open(const uint8_t* filename)
{
	//disable interrupts
	cli();

    //printf("got to rtc open");
	//setting the rtc to mode B
	outb(status_reg_B, rtc_reg_num);
	//reading the current value in the data reg
	unsigned char cur_val = inb(rtc_data);

	outb(status_reg_B, rtc_reg_num);
	//enabling periodic interrupts
	outb(cur_val | MASK_ONE, rtc_data);
	
	uint32_t initial_frequency = 2;
	rtc_write(2,&initial_frequency,0);

	//enable interrupts
	sti(); 
	return 0;
}

uint32_t rtc_write(uint32_t fd, const void *hz, uint32_t nbytes) { 
	unsigned char rate; 
	cli(); 
	//printf("attempting to write to rtc");
	
	if(*((uint32_t * )hz) == TWO_HZ) { 
		rate = (unsigned char) rate_for_2;
	} else if(*((uint32_t * )hz) == FOUR_HZ) { 
		rate = (unsigned char) rate_for_4;
	} else if(*((uint32_t * )hz) == EIGHT_HZ) { 
		rate = (unsigned char) rate_for_8;
	} else if(*((uint32_t * )hz) == SIXTEEN_HZ) { 
		rate = (unsigned char) rate_for_16;
	} else if(*((uint32_t * )hz) == THIRTY_TWO_HZ) { 
		rate = (unsigned char) rate_for_32;
	} else if(*((uint32_t * )hz) == SIXTY_FOUR_HZ) { 
		rate = (unsigned char) rate_for_64;
	} else if(*((uint32_t * )hz) == ONE_TWENTY_EIGHT_HZ) { 
		rate = (unsigned char) rate_for_128;
	} else if(*((uint32_t * )hz) == TWO_FIFTY_SIX_HZ) { 
		rate = (unsigned char) rate_for_256;
	} else if(*((uint32_t * )hz) == FIVE_TWELVE_HZ) { 
		rate = (unsigned char) rate_for_512;
	} else if(*((uint32_t * )hz) == ONE_THOUSAND_TWENTY_FOUR_HZ) { 
		rate = (unsigned char) rate_for_1024;
	} else { 
		rate = BASE_RATE; 
	}

	// SOURCE: OSDEV
	rate &= MASK_TWO;// 0x0F;			
	outb(status_reg_A,  rtc_reg_num);		
	char prev = inb(rtc_data);	
	outb(status_reg_A, rtc_reg_num);		
	outb((prev & MASK_THREE) | rate, rtc_data);  //mask out lower four bits of previous 
	
	sti(); 

	return 0; 

}

uint32_t rtc_read(uint32_t fd, const void* buf, uint32_t nbytes) { 
	sti();
	rtc_interrupt_happened = 0; 
	while(rtc_interrupt_happened == 0);// printf("waiting for rtc_interruptke"); 
	return 0; 
}

uint32_t rtc_close(uint32_t fd) { 
	return 0; 
}



