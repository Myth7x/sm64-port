#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void actor_manager_init(void);
void actor_manager_update(void);
void actor_manager_render(void);
void actor_manager_shutdown(void);

#ifdef __cplusplus
}
#endif
