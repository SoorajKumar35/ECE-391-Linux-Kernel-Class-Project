/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */


uint8_t cached_A1;
uint8_t cached_21;

uint8_t full_mask = 0xff;
/* Initialize the 8259 PIC */
/* FUNCTION: i8259_init 
   INPUTS:    
   OUTPUTS:   
*/ 
void
i8259_init(void)
{
    /* Send all the ICWs to the PIC */
    outb(ICW1, MASTER_8259_PORT); //write ICW1 to MASTER PIC port 
    outb(ICW1, SLAVE_8259_PORT); //write ICW to SLAVE PIC port 

    outb(ICW2_MASTER, MASTER_8259_PORT + 1);
    outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);

    outb(ICW3_MASTER, MASTER_8259_PORT + 1); 
    outb(ICW3_SLAVE, SLAVE_8259_PORT + 1); 

    outb(ICW4, MASTER_8259_PORT + 1); 
    outb(ICW4, SLAVE_8259_PORT + 1); 

    master_mask = 0xFF; 
    slave_mask = 0xFF; 

    printf("8259 INITIALIZED\n");

 //   outb(master_mask, MASTER_8259_PORT);
  //  outb(slave_mask, SLAVE_8259_PORT);

//    enable_irq(SLAVE_IRQ_NUM); 

    
}

/* Enable (unmask) the specified IRQ */
/* FUNCTION: enable_irq 
   INPUTS:    interrupt num
   OUTPUTS:   
*/ 
void
enable_irq(uint32_t irq_num)
{

    /* 
      temp_mask  0100 0000 
      master_mask  1111 1111 

      master_mask 1011 1111 

      temp_mask  0010 0000 
      master_mask 1011 1111 
      master_mask  1001 1111  

        int mask = (mask << irq_num);  
        master_mask = mask ^ master_mask 
      
    */ 

        
    if(irq_num < 0 || irq_num > MAX_PORT_NUM_SLAVE) { 
            return; 
    } else if(irq_num >= 0 && irq_num <= MAX_PORT_NUM_MASTER) { 
        int temp_master_mask = 0x01 << irq_num; //shift one over by the specified IRQ_NUM 
        master_mask = temp_master_mask ^ master_mask;    //clear the bit (interrupt) we want 
    } else if(irq_num >= (MAX_PORT_NUM_MASTER + 1) && irq_num <= MAX_PORT_NUM_SLAVE) { 
        int temp_slave_mask = 0x01 << (irq_num - (MAX_PORT_NUM_MASTER + 1)); 
        slave_mask = temp_slave_mask ^ slave_mask; 
        enable_irq(SLAVE_IRQ_NUM); //enable the master PIC 
    }

    outb(master_mask, MASTER_8259_PORT + 1); 
    outb(slave_mask, SLAVE_8259_PORT + 1);
}

/* Disable (mask) the specified IRQ */
/* FUNCTION: disable_irq 
   INPUTS:    interrupt num
   OUTPUTS:   
*/ 
void
disable_irq(uint32_t irq_num)
{

 
}

/* Send end-of-interrupt signal for the specified IRQ */
/* FUNCTION: send_eoi 
   INPUTS:    interrupt num
   OUTPUTS:   
*/ 
void
send_eoi(uint32_t irq_num)
{
	if(irq_num >= 8)
	   {
		outb(EOI | (irq_num - 8) , SLAVE_8259_PORT);
    outb(EOI | 0x02, MASTER_8259_PORT);
	 } else { 
	    outb(EOI | irq_num, MASTER_8259_PORT);
     }
}

