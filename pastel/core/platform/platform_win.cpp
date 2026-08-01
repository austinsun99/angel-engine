#include "platform_win.h"

#include <libloaderapi.h>
#include <minwindef.h>
#include <windef.h>
#include <windows.h>

#include <cstdint>

#include "core/platform/platform.h"
#include "input.h"

static LRESULT window_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

struct InternalState {
    const char *WINDOW_CLASS = "Pastel Main Window Class";

    HINSTANCE instance;
};

WindowState init_window_win() {
    InternalState *internal = new InternalState();
    WindowState state       = {.running = true, .width = 0, .height = 0, .internal_state = internal};
    return state;
}

bool open_window_win(const WindowConfig config, WindowState *const state) {
    InternalState *internal = static_cast<InternalState *>(state->internal_state);

    if (!GetModuleHandleEx(0, nullptr, &internal->instance)) return false;

    WNDCLASSEX wnd_class = WNDCLASSEX{
        .cbSize      = sizeof(WNDCLASSEX),
        .style       = CS_DBLCLKS,
        .lpfnWndProc = window_callback,
        .cbClsExtra  = 0,
        .cbWndExtra  = 0,
        .hInstance   = internal->instance,
        .hIcon       = LoadIcon(nullptr, IDI_APPLICATION),
        .hCursor     = LoadCursor(nullptr, IDC_ARROW),

        // this is handled in window callback in WM_ERASEBKGND. we set this to null to indicate we paint our own
        // background
        .hbrBackground = nullptr,
        .lpszMenuName  = nullptr,
        .lpszClassName = internal->WINDOW_CLASS,
        .hIconSm       = LoadIcon(nullptr, IDI_APPLICATION),
    };

    const DWORD ex_wnd_style = WS_EX_OVERLAPPEDWINDOW;
    // this is equivalent to WS_OVERLAPPEDWINDOW
    const DWORD wnd_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

    RECT border_rect = {};
    // @hack: the MSDN documentation specifies WS_OVERLAPPED cannot be specified
    AdjustWindowRectEx(&border_rect, wnd_style, FALSE, ex_wnd_style);

    const int winx       = config.x;
    const int winy       = config.y;
    const int win_width  = config.width + border_rect.right - border_rect.left;
    const int win_height = config.height + border_rect.bottom - border_rect.top;

    if (RegisterClassEx(&wnd_class) == 0) return false;

    HWND hwnd = CreateWindowEx(ex_wnd_style,
                               internal->WINDOW_CLASS,
                               config.application_name,
                               wnd_style,

                               // size and positioning
                               winx,
                               winy,
                               win_width,
                               win_height,

                               nullptr,  // parent window
                               nullptr,  // menu
                               internal->instance,
                               nullptr);

    if (hwnd == nullptr) {
        // @todo: obtain error with GetLastError
        return false;
    }

    ShowWindow(hwnd, SW_SHOW);
    return true;
}

bool pump_window_win(WindowState *const state) {
    MSG msg;
    const UINT msg_filter_min = 0;
    const UINT msg_filter_max = 0;
    while (PeekMessage(&msg, nullptr, msg_filter_min, msg_filter_max, PM_REMOVE) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

void deinit_window_win(WindowState *const state) {
    delete static_cast<InternalState *>(state->internal_state);
}

static LRESULT window_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            const bool pressed = msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN;
            const Keycode key  = static_cast<Keycode>(wparam);

            process_key(key, pressed);
        } break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            const bool pressed       = msg == WM_LBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_RBUTTONDOWN;
            MouseButton mouse_button = MouseButton::MAX_BUTTONS;
            switch (msg) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                    mouse_button = MouseButton::LEFT;
                    break;
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                    mouse_button = MouseButton::MIDDLE;
                    break;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                    mouse_button = MouseButton::RIGHT;
                    break;
            }

            process_mouse_button(mouse_button, pressed);
        } break;
        case WM_MOUSEMOVE: {
            // @todo
        } break;
        case WM_ERASEBKGND:
            // A nonzero return value to indicate the program handles erasing the background.
            return 1;
        case WM_CLOSE: {
            const WORD system_language = 0;
            if (MessageBoxEx(hwnd, "Quit program?", "Quit", MB_OKCANCEL, system_language) == IDOK) {
                // @todo: halt execution of program on destroy window.
                // @todo: include some callback before complete application shutdown.
                DestroyWindow(hwnd);
            }
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}
