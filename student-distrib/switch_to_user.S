#define ASM 1
#include "syscalls.h"
#include "x86_desc.h"
#
#define stack_segment 0x83FFFF0 
#define KERNEL_CS 0x0010
#define KERNEL_DS 0x0018
#define USER_CS 0x0023
#define USER_DS 0x002B
#define KERNEL_TSS 0x0030
#define KERNEL_LDT 0x0038
#define FLAG_SETTINGS 0x4200
#
#
# FUNCTION: switch_to_user  
# PURPOSE: complete context switch from kernel space to user space 
# RETURNS: void
#
#
#
#
#
.global switch_to_user

switch_to_user:

	 movw $USER_DS, %ax
     movw %ax, %ds
     movw %ax, %es
     movw %ax, %fs
     movw %ax, %gs #we don't need to worry about SS. it's handled by iret
 
     movl 4(%esp), %ebx
     pushl $USER_DS #user data segment with bottom 2 bits set for ring 3
     pushl $stack_segment

     pushfl
     movl (%esp), %eax
     orl $FLAG_SETTINGS, %eax 
     movl %eax, (%esp)

     pushl $USER_CS #user code segment with bottom 2 bits set for ring 3
     pushl %ebx #may need to remove the _ for this to work right 
     iret



	


