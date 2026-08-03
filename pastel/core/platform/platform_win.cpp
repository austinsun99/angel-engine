#include <profileapi.h>
#include <winnt.h>
#include "defines.h"
#ifdef PLATFORM_WINDOWS

#    include <libloaderapi.h>
#    include <minwindef.h>
#    include <windef.h>
#    include <windows.h>
#    include <windowsx.h>

#    include "core/io/terminal_colours.h"
#    include "core/platform/platform.h"
#    include "input.h"
#    include "platform.h"

static LRESULT window_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

namespace Pastel {
struct InternalState {
    const char *WINDOW_CLASS = "Pastel Main Window Class";
    CONSOLE_SCREEN_BUFFER_INFO initial_console_screen_buf_info;

    HINSTANCE instance;
    HANDLE console_stdout;

    double clock_frequency_inverse;
};

WindowState::WindowState() {
    InternalState *internal = new InternalState();
    input                   = Input();
    running                 = true;
    width                   = 0;
    height                  = 0;
    internal_state          = internal;
}

WindowState::~WindowState() {
    delete static_cast<InternalState *>(internal_state);
}

void WindowState::update() {
    prev_time    = current_time;
    current_time = get_time();
}

bool WindowState::open_window(const WindowConfig config) {
    InternalState *internal = static_cast<InternalState *>(internal_state);

    internal->console_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (internal->console_stdout == INVALID_HANDLE_VALUE) {
        // @todo: error message
    }

    GetConsoleScreenBufferInfo(internal->console_stdout, &internal->initial_console_screen_buf_info);

    LARGE_INTEGER clock_frequency;
    QueryPerformanceFrequency(&clock_frequency);
    internal->clock_frequency_inverse = 1.0 / static_cast<double>(clock_frequency.QuadPart);

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
                               this);

    if (hwnd == nullptr) {
        // @todo: obtain error with GetLastError
        return false;
    }

    ShowWindow(hwnd, SW_SHOW);
    return true;
}

bool WindowState::pump_window() {
    // @todo: set running to false on window post quit message
    MSG msg;
    const UINT msg_filter_min = 0;
    const UINT msg_filter_max = 0;
    while (PeekMessage(&msg, nullptr, msg_filter_min, msg_filter_max, PM_REMOVE) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

constexpr int terminal_colour_to_id(Io::TerminalColour colour) {
    switch (colour) {
        case Io::TERMINAL_COLOUR_BLACK:
        case Io::TERMINAL_COLOUR_MAX:
        case Io::TERMINAL_COLOUR_NONE:
        case Io::TERMINAL_COLOUR_GRAY:
            return 0;
        case Io::TERMINAL_COLOUR_BLUE:
            return 1;
        case Io::TERMINAL_COLOUR_GREEN:
            return 2;
        case Io::TERMINAL_COLOUR_LIGHTBLUE:
            return 3;
        case Io::TERMINAL_COLOUR_RED:
            return 4;
        case Io::TERMINAL_COLOUR_PURPLE:
            return 5;
        case Io::TERMINAL_COLOUR_YELLOW:
            return 6;
        case Io::TERMINAL_COLOUR_LIGHTGRAY:  // note: set light gray the same as white
        case Io::TERMINAL_COLOUR_WHITE:
            return 7;
    }

    return 0;
}

void WindowState::print_terminal_raw(const char *msg) {
    InternalState *state = static_cast<InternalState *>(internal_state);
    DWORD num_written    = 0;
    WriteConsole(state->console_stdout, msg, strlen(msg), &num_written, nullptr);
}

void WindowState::print_terminal(const char *msg, Io::TerminalColour fg, Io::TerminalColour bg) {
    int colour_id        = terminal_colour_to_id(bg) * 16 + terminal_colour_to_id(fg);
    InternalState *state = static_cast<InternalState *>(internal_state);
    SetConsoleTextAttribute(state->console_stdout, colour_id);
    print_terminal_raw(msg);
}

void WindowState::clear_terminal_colour() {
    InternalState *state = static_cast<InternalState *>(internal_state);
    SetConsoleTextAttribute(state->console_stdout, state->initial_console_screen_buf_info.wAttributes);
}

double WindowState::get_time() {
    InternalState *internal = static_cast<InternalState *>(internal_state);
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return static_cast<double>(counter.QuadPart) * internal->clock_frequency_inverse;
}

double WindowState::get_delta_time() {
    return current_time - prev_time;
}

}  // namespace Pastel

using namespace Pastel;
static LRESULT window_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    WindowState *state;
    if (msg == WM_CREATE) {
        CREATESTRUCT *create = reinterpret_cast<CREATESTRUCT *>(lparam);
        state                = reinterpret_cast<WindowState *>(create->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
        return TRUE;
    }

    LONG_PTR data = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    state         = reinterpret_cast<WindowState *>(data);

    switch (msg) {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            const bool pressed       = msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN;
            const Input::Keycode key = static_cast<Input::Keycode>(wparam);

            state->input.process_key(key, pressed);
        } break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            const bool pressed              = msg == WM_LBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_RBUTTONDOWN;
            Input::MouseButton mouse_button = Input::MouseButton::MAX_BUTTONS;
            switch (msg) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                    mouse_button = Input::MouseButton::MOUSE_LEFT;
                    break;
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                    mouse_button = Input::MouseButton::MOUSE_MIDDLE;
                    break;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                    mouse_button = Input::MouseButton::MOUSE_RIGHT;
                    break;
            }

            state->input.process_mouse_button(mouse_button, pressed);
        } break;
        case WM_MOUSEMOVE: {
            int x = GET_X_LPARAM(lparam);
            int y = GET_Y_LPARAM(lparam);
            state->input.process_mouse_position(x, y);
            // @todo
        } break;
        case WM_MOUSEWHEEL: {
            state->input.process_mouse_wheel(GET_WHEEL_DELTA_WPARAM(wparam));
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

#endif
