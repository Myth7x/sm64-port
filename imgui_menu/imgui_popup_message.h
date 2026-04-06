#pragma once
#include "toast_c.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void imgui_popup_message_push(const char *title, const char *body) {
    toast_push(title, body);
}

#ifdef __cplusplus
}
#endif

