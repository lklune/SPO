#ifndef SCHEDULER_BASE_H
#define SCHEDULER_BASE_H

#include "platform_runtime.h"

#define SCHEDULER_MAX_TASKS 64

typedef enum scheduler_policy {
    SCHED_POLICY_FCFS = 0,
    SCHED_POLICY_ROUND_ROBIN = 1,
    SCHED_POLICY_PRIORITY = 2
} scheduler_policy_t;

typedef enum task_state {
    TASK_STATE_NEW = 0,
    TASK_STATE_READY = 1,
    TASK_STATE_RUNNING = 2,
    TASK_STATE_WAITING = 3,
    TASK_STATE_DONE = 4
} task_state_t;

typedef struct scheduler_config {
    scheduler_policy_t policy;
    unsigned quantum_ms;
    unsigned max_tasks;
} scheduler_config_t;

typedef struct task_desc {
    int id;
    const char* name;
    task_state_t state;
    unsigned priority;
    unsigned total_ticks;
    unsigned remaining_ticks;
    unsigned workload_ticks;
} task_desc_t;

typedef struct task_queue {
    task_desc_t* items[SCHEDULER_MAX_TASKS];
    unsigned head;
    unsigned tail;
    unsigned count;
} task_queue_t;

typedef struct scheduler {
    scheduler_config_t config;
    task_desc_t* tasks;
    unsigned task_count;
    task_queue_t ready_queue;
    task_desc_t* current_task;
} scheduler_t;

const char* scheduler_policy_name(scheduler_policy_t policy);
const char* task_state_name(task_state_t state);
void task_init(task_desc_t* task,
    int id,
    const char* name,
    unsigned priority,
    unsigned total_ticks,
    task_state_t initial_state);
void task_queue_init(task_queue_t* queue);
int task_queue_push(task_queue_t* queue, task_desc_t* task);
task_desc_t* task_queue_pop(task_queue_t* queue);
int task_queue_is_empty(const task_queue_t* queue);
int scheduler_init(scheduler_t* scheduler,
    const scheduler_config_t* config,
    task_desc_t* tasks,
    unsigned task_count);
void scheduler_reset_ready_queue(scheduler_t* scheduler);
task_desc_t* scheduler_pick_next(scheduler_t* scheduler);
void scheduler_print_base_info(const platform_runtime_info_t* runtime_info,
    const scheduler_config_t* config,
    const task_desc_t* tasks,
    unsigned task_count);
void scheduler_print_runtime_state(const scheduler_t* scheduler);

#endif
