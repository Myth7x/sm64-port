#if defined(ENABLE_OPENGL) && defined(TARGET_LINUX)

#include <SDL2/SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "gfx_sdl2_imgui.h"
#include "imgui_menu/imgui_menu.h"

void imgui_sdl2_init(void *sdl_window) {
    imgui_menu_init();
    SDL_Window   *wnd = static_cast<SDL_Window *>(sdl_window);
    SDL_GLContext  ctx = SDL_GL_GetCurrentContext();
    ImGui_ImplSDL2_InitForOpenGL(wnd, ctx);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void imgui_sdl2_process_event(void *sdl_event) {
    ImGui_ImplSDL2_ProcessEvent(static_cast<SDL_Event *>(sdl_event));
}

void imgui_sdl2_render_frame(void) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    imgui_menu_new_frame();
    imgui_menu_compose_frame();
    imgui_menu_render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void imgui_sdl2_shutdown(void) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    imgui_menu_shutdown();
}

#else

void imgui_sdl2_init(void *) {}
void imgui_sdl2_process_event(void *) {}
void imgui_sdl2_render_frame(void) {}
void imgui_sdl2_shutdown(void) {}

#endif
