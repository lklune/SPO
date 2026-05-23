#include <string.h>

#include "platform_runtime.h"

int platform_runtime_init(platform_runtime_info_t* info) {
    if (!info) {
        return 0;
    }

    memset(info, 0, sizeof(*info));
    info->platform_name = "Windows";
    info->supports_preemptive_timer = 1;
    info->supports_user_context = 1;
    return 1;
}

void platform_runtime_shutdown(void) {
}

const char* platform_runtime_build_target(void) {
    return "windows";
}
