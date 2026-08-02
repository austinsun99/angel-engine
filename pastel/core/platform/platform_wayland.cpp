#include <sys/mman.h>

#include <cstdint>

#include "defines.h"
#ifdef PLATFORM_LINUX

#    include <sys/syscall.h>
#    include <unistd.h>
#    include <wayland-client-core.h>
#    include <wayland-client-protocol.h>

#    include <algorithm>
#    include <cstring>

#    include "core/platform/platform.h"
#    include "platform_wayland.h"
#    include "xdg-shell-client-protocol.h"

struct InternalState {
    struct wl_display *wl_display;
    struct wl_registry *wl_registry;
    struct wl_compositor *wl_compositor;
    struct wl_shm *wl_shm;  // @temp
    struct xdg_wm_base *xdg_wm_base;

    struct wl_surface *wl_surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;
};

const static struct {
    static void ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
        (void)data;
        xdg_wm_base_pong(xdg_wm_base, serial);
    }
    const xdg_wm_base_listener listener = xdg_wm_base_listener{
        .ping = ping,
    };
} xdg_wm_base_listener;

const static struct {
    static void configure(void *data,
                          struct xdg_toplevel *xdg_toplevel,
                          int32_t width,
                          int32_t height,
                          struct wl_array *states) {
        (void)xdg_toplevel;
        (void)states;
        if (width == 0 || height == 0) return;

        WindowState *state = static_cast<WindowState *>(data);
        state->width       = width;
        state->height      = height;
    }
    static void close(void *data, struct xdg_toplevel *xdg_toplevel) {
        (void)xdg_toplevel;
        WindowState *state = static_cast<WindowState *>(data);
        state->running     = false;
    }
    static void configure_bounds(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height) {
        (void)data;
        (void)xdg_toplevel;
        (void)width;
        (void)height;
    }
    static void wm_capabilities(void *data, struct xdg_toplevel *xdg_toplevel, struct wl_array *capabilities) {
        (void)data;
        (void)xdg_toplevel;
        (void)capabilities;
    }
    const xdg_toplevel_listener listener = xdg_toplevel_listener{.configure        = configure,
                                                                 .close            = close,
                                                                 .configure_bounds = configure_bounds,
                                                                 .wm_capabilities  = wm_capabilities};
} xdg_toplevel_listener;

const static struct {
    static void global(void *data,
                       struct wl_registry *wl_registry,
                       uint32_t name,
                       const char *interface,
                       uint32_t version) {
        (void)wl_registry;
        WindowState *state      = static_cast<WindowState *>(data);
        InternalState *internal = static_cast<InternalState *>(state->internal_state);

        if (strcmp(interface, wl_compositor_interface.name) == 0) {
            const uint32_t min_ver  = std::min<uint32_t>(7, version);
            internal->wl_compositor = static_cast<wl_compositor *>(
                wl_registry_bind(internal->wl_registry, name, &wl_compositor_interface, min_ver));
        } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
            const uint32_t min_ver = std::min<uint32_t>(7, version);
            internal->xdg_wm_base  = static_cast<xdg_wm_base *>(
                wl_registry_bind(internal->wl_registry, name, &xdg_wm_base_interface, min_ver));
            xdg_wm_base_add_listener(internal->xdg_wm_base, &xdg_wm_base_listener.listener, state);
        }

        // @temp remove after vulkan buffers
        if (strcmp(interface, wl_shm_interface.name) == 0) {
            const uint32_t min_ver = std::min<uint32_t>(3, version);
            internal->wl_shm =
                static_cast<wl_shm *>(wl_registry_bind(internal->wl_registry, name, &wl_shm_interface, min_ver));
        }
    }

    static void global_remove(void *data, struct wl_registry *wl_registry, uint32_t name) {
        (void)data;
        (void)wl_registry;
        (void)name;
        return;
    }

    const wl_registry_listener listener = wl_registry_listener{
        .global        = global,
        .global_remove = global_remove,
    };
} wl_registry_listener;

WindowState init_window_wayland() {
    return WindowState{
        .running        = true,
        .width          = 0,
        .height         = 0,
        .internal_state = new InternalState(),
    };
};

bool open_window_wayland(const WindowConfig config, WindowState *const state) {
    InternalState *internal = static_cast<InternalState *>(state->internal_state);

    internal->wl_display = wl_display_connect(nullptr);
    if (internal->wl_display == nullptr) {
        // @todo: log error message
        return false;
    }
    internal->wl_registry = wl_display_get_registry(internal->wl_display);
    wl_registry_add_listener(internal->wl_registry, &wl_registry_listener.listener, state);
    wl_display_roundtrip(internal->wl_display);
    internal->wl_surface = wl_compositor_create_surface(internal->wl_compositor);

    internal->xdg_surface  = xdg_wm_base_get_xdg_surface(internal->xdg_wm_base, internal->wl_surface);
    internal->xdg_toplevel = xdg_surface_get_toplevel(internal->xdg_surface);
    xdg_toplevel_set_title(internal->xdg_toplevel, config.application_name);
    xdg_toplevel_add_listener(internal->xdg_toplevel, &xdg_toplevel_listener.listener, state);

    // @temp
    // the code below allocates a shared memory buffer. This is temporary to get something to show on the screen. Remove
    // after vulkan renderer.
    const int stride = config.width * 4;
    const int size   = stride * config.height;

    int fd = syscall(SYS_memfd_create, "buf", 0);
    ftruncate(fd, size);
    uint8_t *pool_data = static_cast<uint8_t *>(mmap(nullptr, size * 2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    (void)pool_data;

    wl_shm_pool *pool = wl_shm_create_pool(internal->wl_shm, fd, size);
    wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0, config.width, config.height, stride, WL_SHM_FORMAT_XRGB8888);

    wl_surface_attach(internal->wl_surface, buffer, 0, 0);
    wl_surface_commit(internal->wl_surface);

    return true;
}

bool pump_window_wayland(WindowState *const state) {
    const InternalState *internal = static_cast<InternalState *>(state->internal_state);
    wl_display_dispatch_pending(internal->wl_display);
    wl_display_flush(internal->wl_display);
    return true;
}

void deinit_window_wayland(WindowState *const state) {
    InternalState *internal = static_cast<InternalState *>(state->internal_state);
    xdg_toplevel_destroy(internal->xdg_toplevel);
    xdg_surface_destroy(internal->xdg_surface);

    wl_surface_destroy(internal->wl_surface);
    // Obtained from registry global callback
    xdg_wm_base_destroy(internal->xdg_wm_base);
    wl_compositor_destroy(internal->wl_compositor);

    wl_registry_destroy(internal->wl_registry);
    wl_display_disconnect(internal->wl_display);

    delete internal;
}

#endif
