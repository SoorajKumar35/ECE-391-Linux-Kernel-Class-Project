#define ASM 1
#include "syscalls.h"

#define MAX_SYSTEM_CALLS 10 
#define MIN_SYSTEM_CALLS 1 

#
#
# FUNCTION: syscall_handler 
# PURPOSE: handles system call from user-space by jumping to the appropriate function using syscalls jumptable (below) 
# RETURNS: -1 in %eax if failure
#
#
#
.global syscall_handler 


syscall_handler: 
	cli
 
	# Check if the syscall number is out of bounds
	cmpl $0x1, %eax
	jl oob_error
	cmpl $0xA, %eax
	jg oob_error

	pushl %edx
	pushl %ecx
	pushl %ebx

	# Call the appropriate system call
	call *syscalls(,%eax,4)
    xorl %ebx, %ebx 
	je FIN          

	oob_error:
		movl $-1, %eax
		iret

	FIN: 
	popl %ebx 
	popl %ecx 
	popl %edx 
	#sti
	#movl $1, %eax
	iret 

	syscalls: .long 0, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
	.end

