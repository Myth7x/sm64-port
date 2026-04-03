#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void sm64_queue_init(void);
void sm64_queue_shutdown(void);
void sm64_queue_send_mario_state(void);

#ifdef __cplusplus
}
#endif
