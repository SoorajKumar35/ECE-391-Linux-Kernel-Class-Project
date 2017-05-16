#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "syscalls.h"
#include "types.h"

//initialize PIT
void init_PIT(void);
void inter_PIT(void);
void switch_active_task(int term);

extern uint32_t currently_running;

#endif