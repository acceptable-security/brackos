#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <arch/i386/task.h>

void scheduler_advance();
void scheduler_init(task_t* first);

#endif
