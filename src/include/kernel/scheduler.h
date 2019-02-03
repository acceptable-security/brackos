#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <kernel/task.h>

void scheduler_advance();
void scheduler_add(task_t* task);
void scheduler_init(task_t* first);

#ifndef _IGNORE_CURRENT_TASK
extern task_t* current_task;
#endif

#endif
