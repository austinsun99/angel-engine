#include "defines.h"
#ifdef PLATFORM_WINDOWS

#    include <libloaderapi.h>
#    include <minwindef.h>
#    include <windef.h>
#    include <windows.h>
#    include <windowsx.h>
#    include <winnt.h>
#    include <errhandlingapi.h>
#    include <cstring>

#    include "core/io/terminal_colours.h"
#    include "core/platform/platform.h"
#    include "core/logging/logger.h"
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

// returns: dynamically allocated string
static void log_win_error_msg_fatal(DWORD error) {
    LPSTR msg_buf = nullptr;
    size_t size =
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      nullptr,
                      error,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      reinterpret_cast<LPSTR>(&msg_buf),  // :skull:
                      0,
                      nullptr);

    // @todo: replace with string class
    char *msg = new char[size];
    std::memcpy(msg, msg_buf, size);
    LocalFree(msg_buf);
    CORE_LOG_FATAL(msg);
    delete[] msg;
}

WindowState::WindowState() {
    InternalState *internal = new InternalState();
    input                   = Input();
    running                 = true;
    width                   = 0;
    height                  = 0;
    internal_state          = internal;

    internal->console_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (internal->console_stdout == INVALID_HANDLE_VALUE) {
        MessageBox(nullptr, "Unable to obtain STD_OUTPUT_HANDLE", TEXT("Error"), MB_OK);
    }

    if (GetConsoleScreenBufferInfo(internal->console_stdout, &internal->initial_console_screen_buf_info) == FALSE) {
        MessageBox(nullptr, "Unable to obtain ScreenBufferInfo", TEXT("Error"), MB_OK);
    };

    LARGE_INTEGER clock_frequency;
    if (QueryPerformanceFrequency(&clock_frequency) == 0) {
        // This should be unreachable on Windows XP systems and later
        MessageBox(nullptr, "Unable to obtain high-resolution performance counter", TEXT("Error"), MB_OK);
    };
    internal->clock_frequency_inverse = 1.0 / static_cast<double>(clock_frequency.QuadPart);
    console_initialized               = true;
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

    CORE_LOG_INFO("Starting open window process...");
    if (GetModuleHandleEx(0, nullptr, &internal->instance) == 0) {
        log_win_error_msg_fatal(GetLastError());
        return false;
    }
    CORE_LOG_INFO("Windows (1/3): Obtained module handle")

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

    RECT border_rect = {
        .left   = 0,
        .top    = 0,
        .right  = 0,
        .bottom = 0,
    };

    // @hack: the MSDN documentation specifies WS_OVERLAPPED cannot be specified
    if (AdjustWindowRectEx(&border_rect, wnd_style, FALSE, ex_wnd_style) == false) {
        CORE_LOG_ERROR(
            "Unable to obtain window rect. Defaulting to a rect of 0, 0, 0, 0. This may result in a window that is "
            "chopped off the screen");
    }

    const int winx       = config.x;
    const int winy       = config.y;
    const int win_width  = config.width + border_rect.right - border_rect.left;
    const int win_height = config.height + border_rect.bottom - border_rect.top;

    if (RegisterClassEx(&wnd_class) == FALSE) {
        log_win_error_msg_fatal(GetLastError());
        return false;
    }

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
    CORE_LOG_INFO("Windows (2/3): Window has been created")

    if (hwnd == nullptr) {
        log_win_error_msg_fatal(GetLastError());
        return false;
    }

    CORE_LOG_INFO("Windows (3/3): Showing window...")
    ShowWindow(hwnd, SW_SHOW);
    return true;
}

bool WindowState::pump_window() {
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

void WindowState::on_window_close() {
    CORE_LOG_INFO("Windows: closing window")
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

bool WindowState::console_is_initialized() {
    return console_initialized;
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
                state->on_window_close();
                state->running = false;
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
