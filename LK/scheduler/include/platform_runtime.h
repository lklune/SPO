#ifndef SCHEDULER_PLATFORM_RUNTIME_H
#define SCHEDULER_PLATFORM_RUNTIME_H

typedef struct platform_runtime_info {
    const char* platform_name;
    int supports_preemptive_timer;
    int supports_user_context;
} platform_runtime_info_t;

int platform_runtime_init(platform_runtime_info_t* info);
void platform_runtime_shutdown(void);
const char* platform_runtime_build_target(void);

#endif
