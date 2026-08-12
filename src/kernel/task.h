#ifndef TASK_H
#define TASK_H
#include <stdint.h>

#define MAX_TASKS 8
#define TASK_STACK_SIZE 4096

typedef struct task {
    uint32_t esp;
    uint8_t* stack_base;
    int used;
    const char* name;
} task_t;

void scheduler_init(const char* main_task_name);
int task_create(const char* name, void (*entry)(void));
void scheduler_tick(void);
uint32_t scheduler_task_count(void);
const char* scheduler_current_name(void);
#endif