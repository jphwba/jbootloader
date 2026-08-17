#include "task.h"
#include "heap.h"

extern void task_switch(uint32_t* old_esp_ptr, uint32_t new_esp);

static task_t tasks[MAX_TASKS];
static uint32_t num_tasks = 0;
static uint32_t current_task = 0;

static void task_trampoline(void (*entry)(void)) {
    __asm__ volatile ("sti");
    entry();
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

void scheduler_init(const char* main_task_name) {
    tasks[0].used = 1;
    tasks[0].name = main_task_name;
    tasks[0].esp=0;
    tasks[0].stack_base = 0;
    num_tasks = 1;
    current_task = 0;
}

int task_create(const char* name, void (*entry)(void)) {
    if (num_tasks >= MAX_TASKS) {
        return -1;
    }
    uint8_t* stack = (uint8_t*)kmalloc(TASK_STACK_SIZE);
    if (!stack) {
        return -1;
    }
    uint32_t*sp = (uint32_t*)(stack + TASK_STACK_SIZE);

    *(--sp) = (uint32_t)entry;
    *(--sp) = 0;
    *(--sp) = (uint32_t)task_trampoline;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;

    int idx = (int)num_tasks++;
    tasks[idx].esp = (uint32_t)sp;
    tasks[idx].stack_base = stack;
    tasks[idx].used = 1;
    tasks[idx].name = name;
    return idx;
}

void scheduler_tick(void) {
    if(num_tasks <= 1) {
        return;
    }
    uint32_t prev = current_task;
    uint32_t next = (current_task + 1) % num_tasks;
    if (!tasks[next].used) {
        return;
    }
    current_task = next;
    task_switch(&tasks[prev].esp, tasks[next].esp);
}
uint32_t scheduler_task_count(void) {
    return num_tasks;
}
const char* scheduler_current_name(void) {
    return tasks[current_task].name;
}