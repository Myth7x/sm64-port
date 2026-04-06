#include "../compat.h"

#if defined(ENABLE_OPENGL) && defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#include <GL/glew.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gfx_window_manager_api.h"
#include "gfx_screen_config.h"
#include "../mouse.h"
#include "../../game/fps_mode.h"
#include "../../game/fps_camera.h"

#define GFX_API_NAME "WGL - OpenGL"

typedef BOOL  (WINAPI *PFNWGLSWAPINTERVALEXTPROC)(int interval);
typedef HGLRC (WINAPI *PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC share, const int *attribs);

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_FLAGS_ARB         0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB  0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

static struct {
    HWND   hwnd;
    HDC    hdc;
    HGLRC  hglrc;

    unsigned int width;
    unsigned int height;
    bool is_fullscreen;
    DWORD saved_style;
    RECT  saved_rect;

    bool (*on_key_down)(int scancode);
    bool (*on_key_up)(int scancode);
    void (*on_all_keys_up)(void);
    void (*on_fullscreen_changed)(bool);
} wgl;

/* -----------------------------------------------------------------------
 * Virtual-key → DOS/Linux scancode translation table
 * Uses the same scancode values as gfx_glx.c so keyboard_mapping works
 * ----------------------------------------------------------------------- */
static const uint8_t s_vk_to_scancode[256] = {
    [0x00] = 0x00,
    [VK_ESCAPE]  = 0x01,
    ['1'] = 0x02, ['2'] = 0x03, ['3'] = 0x04, ['4'] = 0x05,
    ['5'] = 0x06, ['6'] = 0x07, ['7'] = 0x08, ['8'] = 0x09,
    ['9'] = 0x0a, ['0'] = 0x0b,
    [VK_OEM_MINUS] = 0x0c, [VK_OEM_PLUS] = 0x0d,
    [VK_BACK]    = 0x0e,
    [VK_TAB]     = 0x0f,
    ['Q'] = 0x10, ['W'] = 0x11, ['E'] = 0x12, ['R'] = 0x13,
    ['T'] = 0x14, ['Y'] = 0x15, ['U'] = 0x16, ['I'] = 0x17,
    ['O'] = 0x18, ['P'] = 0x19,
    [VK_OEM_4]   = 0x1a, [VK_OEM_6]  = 0x1b,
    [VK_RETURN]  = 0x1c,
    [VK_LCONTROL]= 0x1d, [VK_RCONTROL]= 0x11d,
    ['A'] = 0x1e, ['S'] = 0x1f, ['D'] = 0x20, ['F'] = 0x21,
    ['G'] = 0x22, ['H'] = 0x23, ['J'] = 0x24, ['K'] = 0x25,
    ['L'] = 0x26,
    [VK_OEM_1]   = 0x27, [VK_OEM_7]  = 0x28, [VK_OEM_3] = 0x29,
    [VK_LSHIFT]  = 0x2a,
    [VK_OEM_5]   = 0x2b,
    ['Z'] = 0x2c, ['X'] = 0x2d, ['C'] = 0x2e, ['V'] = 0x2f,
    ['B'] = 0x30, ['N'] = 0x31, ['M'] = 0x32,
    [VK_OEM_COMMA] = 0x33, [VK_OEM_PERIOD] = 0x34, [VK_OEM_2] = 0x35,
    [VK_RSHIFT]  = 0x36,
    [VK_MULTIPLY]= 0x37,
    [VK_LMENU]   = 0x38, [VK_RMENU]  = 0x138,
    [VK_SPACE]   = 0x39,
    [VK_CAPITAL] = 0x3a,
    [VK_F1]  = 0x3b, [VK_F2]  = 0x3c, [VK_F3]  = 0x3d, [VK_F4]  = 0x3e,
    [VK_F5]  = 0x3f, [VK_F6]  = 0x40, [VK_F7]  = 0x41, [VK_F8]  = 0x42,
    [VK_F9]  = 0x43, [VK_F10] = 0x44, [VK_F11] = 0x57, [VK_F12] = 0x58,
    [VK_NUMLOCK] = 0x45, [VK_SCROLL]   = 0x46,
    [VK_NUMPAD7] = 0x47, [VK_NUMPAD8]  = 0x48, [VK_NUMPAD9]  = 0x49,
    [VK_SUBTRACT]= 0x4a,
    [VK_NUMPAD4] = 0x4b, [VK_NUMPAD5]  = 0x4c, [VK_NUMPAD6]  = 0x4d,
    [VK_ADD]     = 0x4e,
    [VK_NUMPAD1] = 0x4f, [VK_NUMPAD2]  = 0x50, [VK_NUMPAD3]  = 0x51,
    [VK_NUMPAD0] = 0x52, [VK_DECIMAL]  = 0x53,
    [VK_HOME]    = 0x147, [VK_UP]       = 0x148, [VK_PRIOR]   = 0x149,
    [VK_LEFT]    = 0x14b, [VK_RIGHT]    = 0x14d,
    [VK_END]     = 0x14f, [VK_DOWN]     = 0x150, [VK_NEXT]    = 0x151,
    [VK_INSERT]  = 0x152, [VK_DELETE]   = 0x153,
    [VK_LWIN]    = 0x15b, [VK_RWIN]     = 0x15c, [VK_APPS]    = 0x15d,
};

static int vk_to_scancode(WPARAM vk, LPARAM lparam) {
    bool extended = (lparam & (1 << 24)) != 0;
    if (vk < 256) {
        int sc = s_vk_to_scancode[vk];
        if (sc == 0 && vk != 0) return 0;
        if (extended && sc < 0x100) sc |= 0x100;
        return sc;
    }
    return 0;
}

/* -----------------------------------------------------------------------
 * Relative mouse accumulation via WM_INPUT (raw input)
 * ----------------------------------------------------------------------- */
static bool s_raw_input_registered = false;

static void register_raw_mouse(HWND hwnd) {
    if (s_raw_input_registered) return;
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage     = 0x02;
    rid.dwFlags     = RIDEV_INPUTSINK;
    rid.hwndTarget  = hwnd;
    RegisterRawInputDevices(&rid, 1, sizeof(rid));
    s_raw_input_registered = true;
}

static void unregister_raw_mouse(void) {
    if (!s_raw_input_registered) return;
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage     = 0x02;
    rid.dwFlags     = RIDEV_REMOVE;
    rid.hwndTarget  = NULL;
    RegisterRawInputDevices(&rid, 1, sizeof(rid));
    s_raw_input_registered = false;
}

/* -----------------------------------------------------------------------
 * Fullscreen toggle
 * ----------------------------------------------------------------------- */
static void set_fullscreen(bool on, bool call_callback) {
    if (wgl.is_fullscreen == on) return;
    wgl.is_fullscreen = on;

    if (on) {
        GetWindowRect(wgl.hwnd, &wgl.saved_rect);
        wgl.saved_style = (DWORD)GetWindowLongPtr(wgl.hwnd, GWL_STYLE);

        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfo(MonitorFromWindow(wgl.hwnd, MONITOR_DEFAULTTONEAREST), &mi);
        int mw = mi.rcMonitor.right  - mi.rcMonitor.left;
        int mh = mi.rcMonitor.bottom - mi.rcMonitor.top;

        SetWindowLongPtr(wgl.hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        SetWindowPos(wgl.hwnd, HWND_TOP,
                     mi.rcMonitor.left, mi.rcMonitor.top, mw, mh,
                     SWP_FRAMECHANGED | SWP_NOACTIVATE);
        wgl.width  = (unsigned)mw;
        wgl.height = (unsigned)mh;
    } else {
        int rw = wgl.saved_rect.right  - wgl.saved_rect.left;
        int rh = wgl.saved_rect.bottom - wgl.saved_rect.top;
        SetWindowLongPtr(wgl.hwnd, GWL_STYLE, wgl.saved_style);
        SetWindowPos(wgl.hwnd, NULL,
                     wgl.saved_rect.left, wgl.saved_rect.top, rw, rh,
                     SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOZORDER);
        wgl.width  = (unsigned)rw;
        wgl.height = (unsigned)rh;
    }

    if (call_callback && wgl.on_fullscreen_changed) {
        wgl.on_fullscreen_changed(on);
    }
}

/* -----------------------------------------------------------------------
 * WndProc
 * ----------------------------------------------------------------------- */
static LRESULT CALLBACK wgl_wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_INPUT: {
        if (!is_mouse_captured()) break;
        UINT size = 0;
        GetRawInputData((HRAWINPUT)lp, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
        if (size == 0 || size > 512) break;
        char buf[512];
        if (GetRawInputData((HRAWINPUT)lp, RID_INPUT, buf, &size, sizeof(RAWINPUTHEADER)) != size)
            break;
        RAWINPUT *ri = (RAWINPUT *)buf;
        if (ri->header.dwType == RIM_TYPEMOUSE &&
            !(ri->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)) {
            int dx = (int)ri->data.mouse.lLastX;
            int dy = (int)ri->data.mouse.lLastY;
            if (dx || dy) mouse_accum(dx, dy);
        }
        break;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN: {
        bool repeat = (lp & (1 << 30)) != 0;
        if (repeat) break;
        int sc = vk_to_scancode(wp, lp);
        if (wp == VK_F10 || (wp == VK_RETURN && (lp & (1<<29)))) {
            set_fullscreen(!wgl.is_fullscreen, true);
            break;
        }
        if (wp == 'B') fps_teleport_to_map_center();
        if (wp == 'F') fps_toggle_mode();
        if (wp == 'V') fps_toggle_noclip();
        if (wp == VK_ESCAPE) { release_mouse(); break; }
        if (sc && wgl.on_key_down) wgl.on_key_down(sc);
        break;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP: {
        int sc = vk_to_scancode(wp, lp);
        if (sc && wgl.on_key_up) wgl.on_key_up(sc);
        break;
    }
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
        capture_mouse();
        break;
    case WM_KILLFOCUS:
        if (wgl.on_all_keys_up) wgl.on_all_keys_up();
        release_mouse();
        break;
    case WM_SETFOCUS:
        if (gFPSMode || is_mouse_captured())
            capture_mouse();
        break;
    case WM_SIZE:
        wgl.width  = LOWORD(lp);
        wgl.height = HIWORD(lp);
        break;
    case WM_CLOSE:
        exit(0);
        break;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

static bool wgl_do_capture(void) {
    ShowCursor(FALSE);
    SetCapture(wgl.hwnd);
    RECT r;
    GetClientRect(wgl.hwnd, &r);
    MapWindowPoints(wgl.hwnd, NULL, (POINT *)&r, 2);
    ClipCursor(&r);
    return true;
}

static void wgl_do_release(void) {
    ClipCursor(NULL);
    ReleaseCapture();
    while (ShowCursor(TRUE) < 0);
    (void)unregister_raw_mouse;
}

/* -----------------------------------------------------------------------
 * WGL context creation
 * ----------------------------------------------------------------------- */
static void create_wgl_context(void) {
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR), 1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA, 32, 0,0,0,0,0,0,0,0,0,0,0,0,0,
        24, 8, 0, PFD_MAIN_PLANE, 0, 0,0,0
    };

    int pf = ChoosePixelFormat(wgl.hdc, &pfd);
    if (!pf) { fprintf(stderr, "ChoosePixelFormat failed\n"); exit(1); }
    SetPixelFormat(wgl.hdc, pf, &pfd);

    HGLRC dummy = wglCreateContext(wgl.hdc);
    wglMakeCurrent(wgl.hdc, dummy);

    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    if (wglCreateContextAttribsARB) {
        const int attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };
        HGLRC core = wglCreateContextAttribsARB(wgl.hdc, NULL, attribs);
        if (core) {
            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(dummy);
            dummy = core;
        }
    }

    wglMakeCurrent(wgl.hdc, dummy);
    wgl.hglrc = dummy;

    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT =
        (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    if (wglSwapIntervalEXT) wglSwapIntervalEXT(1);

    glewInit();
}

/* -----------------------------------------------------------------------
 * GfxWindowManagerAPI implementation
 * ----------------------------------------------------------------------- */
static void gfx_wgl_init(const char *game_name, bool start_in_fullscreen) {
    wgl.width  = DESIRED_SCREEN_WIDTH;
    wgl.height = DESIRED_SCREEN_HEIGHT;

    WNDCLASSEXW wc = { sizeof(wc) };
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc   = wgl_wnd_proc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"SM64WGL";
    RegisterClassExW(&wc);

    char title[512];
    snprintf(title, sizeof(title), "%s (%s)", game_name, GFX_API_NAME);
    wchar_t wtitle[512];
    MultiByteToWideChar(CP_UTF8, 0, title, -1, wtitle, 512);

    DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    RECT r = { 0, 0, (LONG)wgl.width, (LONG)wgl.height };
    AdjustWindowRect(&r, style, FALSE);

    wgl.hwnd = CreateWindowExW(0, L"SM64WGL", wtitle, style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        r.right - r.left, r.bottom - r.top,
        NULL, NULL, GetModuleHandle(NULL), NULL);
    if (!wgl.hwnd) { fprintf(stderr, "CreateWindowExW failed\n"); exit(1); }

    wgl.hdc = GetDC(wgl.hwnd);
    create_wgl_context();

    register_raw_mouse(wgl.hwnd);
    mouse_register_capture_callbacks(wgl_do_capture, wgl_do_release);

    if (start_in_fullscreen)
        set_fullscreen(true, false);

    capture_mouse();
}

static void gfx_wgl_set_keyboard_callbacks(
    bool (*on_key_down)(int), bool (*on_key_up)(int), void (*on_all_keys_up)(void)) {
    wgl.on_key_down    = on_key_down;
    wgl.on_key_up      = on_key_up;
    wgl.on_all_keys_up = on_all_keys_up;
}

static void gfx_wgl_set_fullscreen_changed_callback(void (*cb)(bool)) {
    wgl.on_fullscreen_changed = cb;
}

static void gfx_wgl_set_fullscreen(bool enable) {
    set_fullscreen(enable, true);
}

static void gfx_wgl_main_loop(void (*run_one_game_iter)(void)) {
    for (;;) run_one_game_iter();
}

static void gfx_wgl_get_dimensions(uint32_t *w, uint32_t *h) {
    *w = wgl.width;
    *h = wgl.height;
}

static void gfx_wgl_handle_events(void) {
    MSG msg;
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

static bool gfx_wgl_start_frame(void) {
    return true;
}

static void gfx_wgl_swap_buffers_begin(void) {
    SwapBuffers(wgl.hdc);
}

static void gfx_wgl_swap_buffers_end(void) {
}

static double gfx_wgl_get_time(void) {
    return 0.0;
}

struct GfxWindowManagerAPI gfx_wgl = {
    gfx_wgl_init,
    gfx_wgl_set_keyboard_callbacks,
    gfx_wgl_set_fullscreen_changed_callback,
    gfx_wgl_set_fullscreen,
    gfx_wgl_main_loop,
    gfx_wgl_get_dimensions,
    gfx_wgl_handle_events,
    gfx_wgl_start_frame,
    gfx_wgl_swap_buffers_begin,
    gfx_wgl_swap_buffers_end,
    gfx_wgl_get_time,
};

#endif
