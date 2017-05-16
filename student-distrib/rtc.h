#ifndef rtc_h 
#define rtc_h 
#include "lib.h"
#define rtc_reg_num 0x70
#define rtc_data 0x71
#define status_reg_A 0x8A
#define status_reg_B 0x8B
#define status_reg_C 0x8C



#ifndef ASM
extern void rtc_init();
uint32_t rtc_open(const uint8_t* filename);

uint32_t rtc_write(uint32_t fd, const void *hz, uint32_t nbytes);

uint32_t rtc_read(uint32_t fd, const void* buf, uint32_t nbytes);

uint32_t rtc_close(uint32_t fd); 

extern int rtc_interrupt_happened; 

#endif /* ASM */

#endif  
