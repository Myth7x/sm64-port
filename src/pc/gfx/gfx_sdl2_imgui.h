#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void imgui_sdl2_init(void *sdl_window);
void imgui_sdl2_process_event(void *sdl_event);
bool imgui_sdl2_wants_mouse_capture(void);
void imgui_sdl2_render_frame(void);
void imgui_sdl2_shutdown(void);

#ifdef __cplusplus
}
#endif
