#include <stdio.h>
#include <string.h>

#include "scheduler_base.h"

const char* scheduler_policy_name(scheduler_policy_t policy) {
    switch (policy) {
    case SCHED_POLICY_FCFS:
        return "FCFS";
    case SCHED_POLICY_ROUND_ROBIN:
        return "Round Robin";
    case SCHED_POLICY_PRIORITY:
        return "Priority";
    default:
        return "Unknown";
    }
}

const char* task_state_name(task_state_t state) {
    switch (state) {
    case TASK_STATE_NEW:
        return "NEW";
    case TASK_STATE_READY:
        return "READY";
    case TASK_STATE_RUNNING:
        return "RUNNING";
    case TASK_STATE_WAITING:
        return "WAITING";
    case TASK_STATE_DONE:
        return "DONE";
    default:
        return "UNKNOWN";
    }
}

void task_init(task_desc_t* task,
    int id,
    const char* name,
    unsigned priority,
    unsigned total_ticks,
    task_state_t initial_state) {
    if (!task) {
        return;
    }

    task->id = id;
    task->name = name;
    task->state = initial_state;
    task->priority = priority;
    task->total_ticks = total_ticks;
    task->remaining_ticks = total_ticks;
    task->workload_ticks = total_ticks;
}

void task_queue_init(task_queue_t* queue) {
    if (!queue) {
        return;
    }

    memset(queue, 0, sizeof(*queue));
}

int task_queue_push(task_queue_t* queue, task_desc_t* task) {
    if (!queue || !task) {
        return 0;
    }

    if (queue->count >= SCHEDULER_MAX_TASKS) {
        return 0;
    }

    queue->items[queue->tail] = task;
    queue->tail = (queue->tail + 1) % SCHEDULER_MAX_TASKS;
    queue->count++;
    return 1;
}

task_desc_t* task_queue_pop(task_queue_t* queue) {
    task_desc_t* task;

    if (!queue || queue->count == 0) {
        return NULL;
    }

    task = queue->items[queue->head];
    queue->items[queue->head] = NULL;
    queue->head = (queue->head + 1) % SCHEDULER_MAX_TASKS;
    queue->count--;
    return task;
}

int task_queue_is_empty(const task_queue_t* queue) {
    return !queue || queue->count == 0;
}

int scheduler_init(scheduler_t* scheduler,
    const scheduler_config_t* config,
    task_desc_t* tasks,
    unsigned task_count) {
    if (!scheduler || !config || !tasks) {
        return 0;
    }

    if (task_count == 0 || task_count > SCHEDULER_MAX_TASKS || task_count > config->max_tasks) {
        return 0;
    }

    memset(scheduler, 0, sizeof(*scheduler));
    scheduler->config = *config;
    scheduler->tasks = tasks;
    scheduler->task_count = task_count;
    task_queue_init(&scheduler->ready_queue);
    scheduler_reset_ready_queue(scheduler);
    return 1;
}

void scheduler_reset_ready_queue(scheduler_t* scheduler) {
    unsigned i;

    if (!scheduler) {
        return;
    }

    task_queue_init(&scheduler->ready_queue);

    for (i = 0; i < scheduler->task_count; i++) {
        if (scheduler->tasks[i].state == TASK_STATE_READY) {
            task_queue_push(&scheduler->ready_queue, &scheduler->tasks[i]);
        }
    }
}

static task_desc_t* scheduler_pick_priority_task(scheduler_t* scheduler) {
    task_desc_t* best = NULL;
    unsigned i;

    if (!scheduler) {
        return NULL;
    }

    for (i = 0; i < scheduler->task_count; i++) {
        task_desc_t* current = &scheduler->tasks[i];
        if (current->state != TASK_STATE_READY) {
            continue;
        }

        if (!best || current->priority > best->priority) {
            best = current;
        }
    }

    return best;
}

task_desc_t* scheduler_pick_next(scheduler_t* scheduler) {
    task_desc_t* next_task = NULL;

    if (!scheduler) {
        return NULL;
    }

    if (scheduler->config.policy == SCHED_POLICY_PRIORITY) {
        next_task = scheduler_pick_priority_task(scheduler);
    } else {
        next_task = task_queue_pop(&scheduler->ready_queue);
    }

    if (next_task) {
        next_task->state = TASK_STATE_RUNNING;
        scheduler->current_task = next_task;
    }

    return next_task;
}

void scheduler_print_base_info(const platform_runtime_info_t* runtime_info,
    const scheduler_config_t* config,
    const task_desc_t* tasks,
    unsigned task_count) {
    printf("scheduler base\n");
    printf("platform: %s\n", runtime_info ? runtime_info->platform_name : "unknown");
    printf("build target: %s\n", platform_runtime_build_target());
    printf("preemptive timer: %s\n",
        runtime_info && runtime_info->supports_preemptive_timer ? "yes" : "no");
    printf("user context: %s\n",
        runtime_info && runtime_info->supports_user_context ? "yes" : "no");
    printf("policy: %s\n", config ? scheduler_policy_name(config->policy) : "unknown");
    printf("quantum ms: %u\n", config ? config->quantum_ms : 0);
    printf("max tasks: %u\n", config ? config->max_tasks : 0);
    printf("tasks:\n");

    for (unsigned i = 0; i < task_count; i++) {
        printf("  #%d %s state=%s priority=%u total=%u remaining=%u\n",
            tasks[i].id,
            tasks[i].name ? tasks[i].name : "task",
            task_state_name(tasks[i].state),
            tasks[i].priority,
            tasks[i].total_ticks,
            tasks[i].remaining_ticks);
    }
}

void scheduler_print_runtime_state(const scheduler_t* scheduler) {
    unsigned i;

    if (!scheduler) {
        return;
    }

    printf("ready queue count: %u\n", scheduler->ready_queue.count);
    printf("current task: %s\n",
        scheduler->current_task && scheduler->current_task->name
        ? scheduler->current_task->name
        : "none");
    printf("ready queue order:\n");

    for (i = 0; i < scheduler->ready_queue.count; i++) {
        unsigned index = (scheduler->ready_queue.head + i) % SCHEDULER_MAX_TASKS;
        task_desc_t* task = scheduler->ready_queue.items[index];
        if (!task) {
            continue;
        }

        printf("  #%d %s priority=%u remaining=%u\n",
            task->id,
            task->name ? task->name : "task",
            task->priority,
            task->remaining_ticks);
    }
}
