#include "scheduler_base.h"

int main(void) {
    platform_runtime_info_t runtime_info;
    scheduler_config_t config;
    scheduler_t scheduler;
    task_desc_t demo_tasks[3];
    task_desc_t* next_task;

    if (!platform_runtime_init(&runtime_info)) {
        return 1;
    }

    config.policy = SCHED_POLICY_ROUND_ROBIN;
    config.quantum_ms = 25;
    config.max_tasks = 16;

    task_init(&demo_tasks[0], 1, "cpu_task_a", 3, 300, TASK_STATE_READY);
    task_init(&demo_tasks[1], 2, "cpu_task_b", 1, 500, TASK_STATE_READY);
    task_init(&demo_tasks[2], 3, "io_like_task", 5, 120, TASK_STATE_WAITING);

    if (!scheduler_init(&scheduler, &config, demo_tasks, 3)) {
        platform_runtime_shutdown();
        return 1;
    }

    scheduler_print_base_info(&runtime_info, &config, demo_tasks, 3);
    scheduler_print_runtime_state(&scheduler);

    next_task = scheduler_pick_next(&scheduler);
    if (next_task) {
        next_task->remaining_ticks -= next_task->remaining_ticks > 25 ? 25 : next_task->remaining_ticks;
    }

    printf("selected task: %s\n", next_task ? next_task->name : "none");
    scheduler_print_runtime_state(&scheduler);
    platform_runtime_shutdown();
    return 0;
}
