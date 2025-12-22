/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_x11_display.c - X11 backend implementation
 */

#include "venom/backend/venom_display.h"
#include "venom/backend/venom_event.h"
#include "venom/core/venom_memory.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * X11 DISPLAY STRUCTURE
 * ============================================================================ */

typedef struct VenomX11Display {
    VenomDisplay base;  /* Must be first for casting */
    
    Display* xdisplay;
    int default_screen;
    Window root_window;
    
    /* XIM - X Input Method for international text input */
    XIM xim;                    /* Input method handle */
    XIMStyles* xim_styles;      /* Supported input styles */
    
    /* Atoms for protocols */
    Atom wm_delete_window;
    Atom wm_protocols;
    Atom net_wm_name;
    Atom utf8_string;
    
    /* Registered windows (simple array for now) */
    struct {
        Window xwindow;
        VenomU32 id;
        XIC xic;                /* Input context per window */
    } windows[64];
    VenomU32 window_count;
    VenomU32 next_window_id;
    
} VenomX11Display;

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

static void x11_display_destroy(VenomDisplay* display);
static VenomI32 x11_get_screen_count(VenomDisplay* display);
static VenomSize2D x11_get_screen_size(VenomDisplay* display, VenomI32 screen);
static VenomI32 x11_get_default_screen(VenomDisplay* display);
static VenomBool x11_poll_event(VenomDisplay* display, VenomEvent* event_out);
static VenomBool x11_wait_event(VenomDisplay* display, VenomEvent* event_out);
static void x11_flush(VenomDisplay* display);
static VenomResultPtr x11_create_window(VenomDisplay* display, const char* title,
                                         VenomI32 x, VenomI32 y,
                                         VenomU32 width, VenomU32 height);
static VenomResultPtr x11_create_window_typed(VenomDisplay* display,
                                               VenomWindowType type,
                                               VenomWindowPosition position,
                                               const char* title,
                                               VenomU32 width, VenomU32 height);

/* ============================================================================
 * VTABLE
 * ============================================================================ */

static const VenomDisplayOps x11_display_ops = {
    .destroy = x11_display_destroy,
    .get_screen_count = x11_get_screen_count,
    .get_screen_size = x11_get_screen_size,
    .get_default_screen = x11_get_default_screen,
    .poll_event = x11_poll_event,
    .wait_event = x11_wait_event,
    .flush = x11_flush,
    .create_window = x11_create_window,
    .create_window_typed = x11_create_window_typed,
};

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================ */

static VenomU32 x11_find_window_id(VenomX11Display* d, Window xwin) {
    for (VenomU32 i = 0; i < d->window_count; i++) {
        if (d->windows[i].xwindow == xwin) {
            return d->windows[i].id;
        }
    }
    return 0;
}

static VenomKeyCode x11_translate_keysym(KeySym keysym) {
    /* Basic mapping - expand as needed */
    if (keysym >= XK_a && keysym <= XK_z) {
        return (VenomKeyCode)(VENOM_KEY_A + (keysym - XK_a));
    }
    if (keysym >= XK_A && keysym <= XK_Z) {
        return (VenomKeyCode)(VENOM_KEY_A + (keysym - XK_A));
    }
    if (keysym >= XK_0 && keysym <= XK_9) {
        return (VenomKeyCode)(VENOM_KEY_0 + (keysym - XK_0));
    }
    
    switch (keysym) {
        case XK_Escape: return VENOM_KEY_ESCAPE;
        case XK_Return: return VENOM_KEY_RETURN;
        case XK_Tab: return VENOM_KEY_TAB;
        case XK_BackSpace: return VENOM_KEY_BACKSPACE;
        case XK_space: return VENOM_KEY_SPACE;
        case XK_Left: return VENOM_KEY_LEFT;
        case XK_Right: return VENOM_KEY_RIGHT;
        case XK_Up: return VENOM_KEY_UP;
        case XK_Down: return VENOM_KEY_DOWN;
        case XK_Home: return VENOM_KEY_HOME;
        case XK_End: return VENOM_KEY_END;
        case XK_Page_Up: return VENOM_KEY_PAGE_UP;
        case XK_Page_Down: return VENOM_KEY_PAGE_DOWN;
        case XK_Insert: return VENOM_KEY_INSERT;
        case XK_Delete: return VENOM_KEY_DELETE;
        case XK_F1: return VENOM_KEY_F1;
        case XK_F2: return VENOM_KEY_F2;
        case XK_F3: return VENOM_KEY_F3;
        case XK_F4: return VENOM_KEY_F4;
        case XK_F5: return VENOM_KEY_F5;
        case XK_F6: return VENOM_KEY_F6;
        case XK_F7: return VENOM_KEY_F7;
        case XK_F8: return VENOM_KEY_F8;
        case XK_F9: return VENOM_KEY_F9;
        case XK_F10: return VENOM_KEY_F10;
        case XK_F11: return VENOM_KEY_F11;
        case XK_F12: return VENOM_KEY_F12;
        default: return VENOM_KEY_UNKNOWN;
    }
}

static VenomKeyMod x11_translate_modifiers(unsigned int state) {
    VenomKeyMod mods = VENOM_KEYMOD_NONE;
    if (state & ShiftMask) mods |= VENOM_KEYMOD_SHIFT;
    if (state & ControlMask) mods |= VENOM_KEYMOD_CTRL;
    if (state & Mod1Mask) mods |= VENOM_KEYMOD_ALT;
    if (state & Mod4Mask) mods |= VENOM_KEYMOD_SUPER;
    if (state & LockMask) mods |= VENOM_KEYMOD_CAPS;
    if (state & Mod2Mask) mods |= VENOM_KEYMOD_NUM;
    return mods;
}

static VenomBool x11_translate_event(VenomX11Display* d, XEvent* xevent, VenomEvent* out) {
    memset(out, 0, sizeof(*out));
    
    switch (xevent->type) {
        case Expose:
            if (xevent->xexpose.count == 0) {  /* Only on last expose */
                out->type = VENOM_EVENT_WINDOW_EXPOSE;
                out->window.window_id = x11_find_window_id(d, xevent->xexpose.window);
                out->window.width = xevent->xexpose.width;
                out->window.height = xevent->xexpose.height;
                return VENOM_TRUE;
            }
            return VENOM_FALSE;
            
        case ConfigureNotify:
            out->type = VENOM_EVENT_WINDOW_RESIZE;
            out->window.window_id = x11_find_window_id(d, xevent->xconfigure.window);
            out->window.x = xevent->xconfigure.x;
            out->window.y = xevent->xconfigure.y;
            out->window.width = xevent->xconfigure.width;
            out->window.height = xevent->xconfigure.height;
            return VENOM_TRUE;
            
        case FocusIn:
            out->type = VENOM_EVENT_WINDOW_FOCUS_IN;
            out->window.window_id = x11_find_window_id(d, xevent->xfocus.window);
            return VENOM_TRUE;
            
        case FocusOut:
            out->type = VENOM_EVENT_WINDOW_FOCUS_OUT;
            out->window.window_id = x11_find_window_id(d, xevent->xfocus.window);
            return VENOM_TRUE;
            
        case MotionNotify:
            out->type = VENOM_EVENT_MOUSE_MOVE;
            out->mouse.window_id = x11_find_window_id(d, xevent->xmotion.window);
            out->mouse.x = xevent->xmotion.x;
            out->mouse.y = xevent->xmotion.y;
            out->mouse.root_x = xevent->xmotion.x_root;
            out->mouse.root_y = xevent->xmotion.y_root;
            out->mouse.modifiers = x11_translate_modifiers(xevent->xmotion.state);
            return VENOM_TRUE;
            
        case ButtonPress:
        case ButtonRelease:
            if (xevent->xbutton.button == 4 || xevent->xbutton.button == 5 ||
                xevent->xbutton.button == 6 || xevent->xbutton.button == 7) {
                /* Scroll wheel */
                out->type = VENOM_EVENT_MOUSE_SCROLL;
                out->scroll.window_id = x11_find_window_id(d, xevent->xbutton.window);
                out->scroll.x = xevent->xbutton.x;
                out->scroll.y = xevent->xbutton.y;
                if (xevent->xbutton.button == 4) out->scroll.delta_y = -1.0f;
                else if (xevent->xbutton.button == 5) out->scroll.delta_y = 1.0f;
                else if (xevent->xbutton.button == 6) out->scroll.delta_x = -1.0f;
                else if (xevent->xbutton.button == 7) out->scroll.delta_x = 1.0f;
            } else {
                out->type = (xevent->type == ButtonPress) ? 
                            VENOM_EVENT_MOUSE_BUTTON_DOWN : VENOM_EVENT_MOUSE_BUTTON_UP;
                out->mouse.window_id = x11_find_window_id(d, xevent->xbutton.window);
                out->mouse.x = xevent->xbutton.x;
                out->mouse.y = xevent->xbutton.y;
                out->mouse.root_x = xevent->xbutton.x_root;
                out->mouse.root_y = xevent->xbutton.y_root;
                out->mouse.button = (VenomMouseButton)xevent->xbutton.button;
                out->mouse.modifiers = x11_translate_modifiers(xevent->xbutton.state);
            }
            return VENOM_TRUE;
            
        case EnterNotify:
            out->type = VENOM_EVENT_MOUSE_ENTER;
            out->mouse.window_id = x11_find_window_id(d, xevent->xcrossing.window);
            out->mouse.x = xevent->xcrossing.x;
            out->mouse.y = xevent->xcrossing.y;
            return VENOM_TRUE;
            
        case LeaveNotify:
            out->type = VENOM_EVENT_MOUSE_LEAVE;
            out->mouse.window_id = x11_find_window_id(d, xevent->xcrossing.window);
            out->mouse.x = xevent->xcrossing.x;
            out->mouse.y = xevent->xcrossing.y;
            return VENOM_TRUE;
            
        case KeyPress:
        case KeyRelease: {
            KeySym keysym = NoSymbol;
            Status status = 0;
            char buf[64];  /* UTF-8 can be up to 4 bytes per char */
            int len = 0;
            
            /* Find XIC for this window */
            XIC xic = NULL;
            VenomU32 win_id = x11_find_window_id(d, xevent->xkey.window);
            for (VenomU32 i = 0; i < d->window_count; i++) {
                if (d->windows[i].id == win_id) {
                    xic = d->windows[i].xic;
                    break;
                }
            }
            
            /* Use Xutf8LookupString if XIC available for proper UTF-8 */
            if (xic && xevent->type == KeyPress) {
                len = Xutf8LookupString(xic, &xevent->xkey, buf, sizeof(buf) - 1, 
                                         &keysym, &status);
                if (status == XBufferOverflow) {
                    len = 0;  /* Buffer too small */
                }
            } else {
                /* Fallback to XLookupString */
                len = XLookupString(&xevent->xkey, buf, sizeof(buf) - 1, &keysym, NULL);
            }
            buf[len < 63 ? len : 63] = '\0';
            
            out->type = (xevent->type == KeyPress) ? VENOM_EVENT_KEY_DOWN : VENOM_EVENT_KEY_UP;
            out->key.window_id = win_id;
            out->key.key = x11_translate_keysym(keysym);
            out->key.scancode = xevent->xkey.keycode;
            out->key.modifiers = x11_translate_modifiers(xevent->xkey.state);
            out->key.is_repeat = VENOM_FALSE;
            
            /* Store UTF-8 text in event for TEXT_INPUT handling */
            if (xevent->type == KeyPress && len > 0 && (unsigned char)buf[0] >= 32) {
                memcpy(out->text.text, buf, len < 31 ? len + 1 : 31);
                out->text.text[31] = '\0';
            }
            return VENOM_TRUE;
        }
            
        case ClientMessage:
            if ((Atom)xevent->xclient.data.l[0] == d->wm_delete_window) {
                out->type = VENOM_EVENT_WINDOW_CLOSE;
                out->window.window_id = x11_find_window_id(d, xevent->xclient.window);
                return VENOM_TRUE;
            }
            return VENOM_FALSE;
            
        case MapNotify:
            out->type = VENOM_EVENT_WINDOW_SHOW;
            out->window.window_id = x11_find_window_id(d, xevent->xmap.window);
            return VENOM_TRUE;
            
        case UnmapNotify:
            out->type = VENOM_EVENT_WINDOW_HIDE;
            out->window.window_id = x11_find_window_id(d, xevent->xunmap.window);
            return VENOM_TRUE;
            
        default:
            return VENOM_FALSE;
    }
}

/* ============================================================================
 * DISPLAY OPERATIONS
 * ============================================================================ */

static void x11_display_destructor(void* self) {
    VenomX11Display* d = (VenomX11Display*)self;
    if (d->xdisplay) {
        XCloseDisplay(d->xdisplay);
        d->xdisplay = NULL;
    }
}

static void x11_display_destroy(VenomDisplay* display) {
    /* The destructor will be called by venom_unref */
    venom_unref(display);
}

static VenomI32 x11_get_screen_count(VenomDisplay* display) {
    VenomX11Display* d = (VenomX11Display*)display;
    return ScreenCount(d->xdisplay);
}

static VenomSize2D x11_get_screen_size(VenomDisplay* display, VenomI32 screen) {
    VenomX11Display* d = (VenomX11Display*)display;
    Screen* scr = ScreenOfDisplay(d->xdisplay, screen);
    return (VenomSize2D){
        .width = WidthOfScreen(scr),
        .height = HeightOfScreen(scr)
    };
}

static VenomI32 x11_get_default_screen(VenomDisplay* display) {
    VenomX11Display* d = (VenomX11Display*)display;
    return d->default_screen;
}

static VenomBool x11_poll_event(VenomDisplay* display, VenomEvent* event_out) {
    VenomX11Display* d = (VenomX11Display*)display;
    
    while (XPending(d->xdisplay) > 0) {
        XEvent xevent;
        XNextEvent(d->xdisplay, &xevent);
        
        if (x11_translate_event(d, &xevent, event_out)) {
            return VENOM_TRUE;
        }
    }
    
    return VENOM_FALSE;
}

static VenomBool x11_wait_event(VenomDisplay* display, VenomEvent* event_out) {
    VenomX11Display* d = (VenomX11Display*)display;
    
    while (1) {
        XEvent xevent;
        XNextEvent(d->xdisplay, &xevent);
        
        if (x11_translate_event(d, &xevent, event_out)) {
            return VENOM_TRUE;
        }
    }
}

static void x11_flush(VenomDisplay* display) {
    VenomX11Display* d = (VenomX11Display*)display;
    XFlush(d->xdisplay);
}

static VenomResultPtr x11_create_window(VenomDisplay* display, const char* title,
                                         VenomI32 x, VenomI32 y,
                                         VenomU32 width, VenomU32 height) {
    VenomX11Display* d = (VenomX11Display*)display;
    
    if (d->window_count >= 64) {
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    /* Create the window */
    Window xwindow = XCreateSimpleWindow(
        d->xdisplay,
        d->root_window,
        x, y,
        width, height,
        0,  /* border width */
        BlackPixel(d->xdisplay, d->default_screen),
        WhitePixel(d->xdisplay, d->default_screen)
    );
    
    if (xwindow == 0) {
        return VENOM_ERR_PTR(VENOM_ERROR_WINDOW_CREATE);
    }
    
    /* Set window title */
    if (title) {
        XChangeProperty(
            d->xdisplay, xwindow,
            d->net_wm_name, d->utf8_string,
            8, PropModeReplace,
            (unsigned char*)title, strlen(title)
        );
        XStoreName(d->xdisplay, xwindow, title);
    }
    
    /* Register for WM_DELETE_WINDOW */
    XSetWMProtocols(d->xdisplay, xwindow, &d->wm_delete_window, 1);
    
    /* Select input events */
    XSelectInput(d->xdisplay, xwindow,
        ExposureMask | StructureNotifyMask | FocusChangeMask |
        KeyPressMask | KeyReleaseMask |
        ButtonPressMask | ButtonReleaseMask |
        PointerMotionMask | EnterWindowMask | LeaveWindowMask
    );
    
    /* Register in our tracking array */
    VenomU32 id = ++d->next_window_id;
    d->windows[d->window_count].xwindow = xwindow;
    d->windows[d->window_count].id = id;
    
    /* Create XIC (Input Context) for this window if XIM is available */
    if (d->xim) {
        d->windows[d->window_count].xic = XCreateIC(d->xim,
            XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
            XNClientWindow, xwindow,
            XNFocusWindow, xwindow,
            NULL);
    } else {
        d->windows[d->window_count].xic = NULL;
    }
    d->window_count++;
    
    /* Map (show) the window */
    XMapWindow(d->xdisplay, xwindow);
    XFlush(d->xdisplay);
    
    /* Return the X window as pointer (we'll wrap this properly later) */
    return VENOM_OK_PTR((void*)(uintptr_t)xwindow);
}

static VenomResultPtr x11_create_window_typed(VenomDisplay* display,
                                               VenomWindowType type,
                                               VenomWindowPosition position,
                                               const char* title,
                                               VenomU32 width, VenomU32 height) {
    VenomX11Display* d = (VenomX11Display*)display;
    
    if (d->window_count >= 64) {
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    /* Get screen dimensions */
    int screen_width = DisplayWidth(d->xdisplay, d->default_screen);
    int screen_height = DisplayHeight(d->xdisplay, d->default_screen);
    
    /* Calculate position and size based on type/position */
    VenomI32 x = 0, y = 0;
    VenomBool override_redirect = VENOM_FALSE;
    
    switch (position) {
        case VENOM_POSITION_CENTER:
            x = (screen_width - (int)width) / 2;
            y = (screen_height - (int)height) / 2;
            break;
        case VENOM_POSITION_TOP:
            x = 0;
            y = 0;
            width = screen_width;
            break;
        case VENOM_POSITION_BOTTOM:
            x = 0;
            y = screen_height - height;
            width = screen_width;
            break;
        case VENOM_POSITION_FULLSCREEN:
            x = 0;
            y = 0;
            width = screen_width;
            height = screen_height;
            break;
        default:
            x = 0;
            y = 0;
            break;
    }
    
    /* Special handling for window types */
    if (type == VENOM_WINDOW_PANEL) {
        override_redirect = VENOM_TRUE;
        y = 0;
        width = screen_width;
    } else if (type == VENOM_WINDOW_DOCK) {
        y = screen_height - height;
        width = screen_width;
    } else if (type == VENOM_WINDOW_POPUP) {
        override_redirect = VENOM_TRUE;
        /* Position popup at top-right, below panel (38px) */
        x = screen_width - (int)width - 10;
        y = 38 + 8;  /* Below panel height + padding */
    } else if (type == VENOM_WINDOW_LAUNCHER) {
        override_redirect = VENOM_TRUE;
        x = 0;
        y = 0;
        width = screen_width;
        height = screen_height;
    }
    
    /* Create window with attributes */
    XSetWindowAttributes swa = {0};
    swa.event_mask = ExposureMask | StructureNotifyMask | FocusChangeMask |
                     KeyPressMask | KeyReleaseMask |
                     ButtonPressMask | ButtonReleaseMask |
                     PointerMotionMask | EnterWindowMask | LeaveWindowMask;
    swa.override_redirect = override_redirect ? True : False;
    swa.background_pixel = BlackPixel(d->xdisplay, d->default_screen);
    
    unsigned long mask = CWEventMask | CWBackPixel;
    if (override_redirect) {
        mask |= CWOverrideRedirect;
    }
    
    Window xwindow = XCreateWindow(
        d->xdisplay,
        d->root_window,
        x, y,
        width, height,
        0,
        CopyFromParent,
        InputOutput,
        CopyFromParent,
        mask,
        &swa
    );
    
    if (xwindow == 0) {
        return VENOM_ERR_PTR(VENOM_ERROR_WINDOW_CREATE);
    }
    
    /* Set window title */
    if (title) {
        XChangeProperty(d->xdisplay, xwindow, d->net_wm_name, d->utf8_string,
                        8, PropModeReplace, (unsigned char*)title, strlen(title));
        XStoreName(d->xdisplay, xwindow, title);
    }
    
    /* Set EWMH window type */
    Atom type_atom = XInternAtom(d->xdisplay, "_NET_WM_WINDOW_TYPE", False);
    Atom wm_type;
    
    switch (type) {
        case VENOM_WINDOW_PANEL:
        case VENOM_WINDOW_DOCK:
            wm_type = XInternAtom(d->xdisplay, "_NET_WM_WINDOW_TYPE_DOCK", False);
            break;
        case VENOM_WINDOW_POPUP:
            wm_type = XInternAtom(d->xdisplay, "_NET_WM_WINDOW_TYPE_POPUP_MENU", False);
            break;
        case VENOM_WINDOW_LAUNCHER:
            wm_type = XInternAtom(d->xdisplay, "_NET_WM_WINDOW_TYPE_SPLASH", False);
            break;
        case VENOM_WINDOW_DESKTOP:
            wm_type = XInternAtom(d->xdisplay, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
            break;
        default:
            wm_type = XInternAtom(d->xdisplay, "_NET_WM_WINDOW_TYPE_NORMAL", False);
            break;
    }
    XChangeProperty(d->xdisplay, xwindow, type_atom, XA_ATOM, 32,
                    PropModeReplace, (unsigned char*)&wm_type, 1);
    
    /* Set strut for panels/docks */
    if (type == VENOM_WINDOW_PANEL || type == VENOM_WINDOW_DOCK) {
        Atom strut_atom = XInternAtom(d->xdisplay, "_NET_WM_STRUT_PARTIAL", False);
        long strut[12] = {0};
        
        if (type == VENOM_WINDOW_PANEL) {
            /* Top strut */
            strut[2] = height;              /* top */
            strut[8] = 0;                   /* top_start_x */
            strut[9] = screen_width - 1;    /* top_end_x */
        } else {
            /* Bottom strut */
            strut[3] = height;              /* bottom */
            strut[10] = 0;                  /* bottom_start_x */
            strut[11] = screen_width - 1;   /* bottom_end_x */
        }
        
        XChangeProperty(d->xdisplay, xwindow, strut_atom, XA_CARDINAL, 32,
                        PropModeReplace, (unsigned char*)strut, 12);
    }
    
    /* Set always on top for panels */
    if (type == VENOM_WINDOW_PANEL || type == VENOM_WINDOW_DOCK || 
        type == VENOM_WINDOW_LAUNCHER) {
        Atom state_atom = XInternAtom(d->xdisplay, "_NET_WM_STATE", False);
        Atom above_atom = XInternAtom(d->xdisplay, "_NET_WM_STATE_ABOVE", False);
        XChangeProperty(d->xdisplay, xwindow, state_atom, XA_ATOM, 32,
                        PropModeReplace, (unsigned char*)&above_atom, 1);
    }
    
    /* Register for WM_DELETE_WINDOW */
    XSetWMProtocols(d->xdisplay, xwindow, &d->wm_delete_window, 1);
    
    /* Register in tracking array */
    VenomU32 id = ++d->next_window_id;
    d->windows[d->window_count].xwindow = xwindow;
    d->windows[d->window_count].id = id;
    
    /* Create XIC if XIM available */
    if (d->xim) {
        d->windows[d->window_count].xic = XCreateIC(d->xim,
            XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
            XNClientWindow, xwindow,
            XNFocusWindow, xwindow,
            NULL);
    } else {
        d->windows[d->window_count].xic = NULL;
    }
    d->window_count++;
    
    /* Map window */
    XMapWindow(d->xdisplay, xwindow);
    XRaiseWindow(d->xdisplay, xwindow);
    XFlush(d->xdisplay);
    
    return VENOM_OK_PTR((void*)(uintptr_t)xwindow);
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VenomResultPtr venom_display_open(VenomBackendType backend, const char* display_name) {
    /* Currently only X11 is supported */
    if (backend != VENOM_BACKEND_X11 && backend != VENOM_BACKEND_AUTO) {
        return VENOM_ERR_PTR(VENOM_ERROR_NOT_SUPPORTED);
    }
    
    /* Open X11 display */
    Display* xdisplay = XOpenDisplay(display_name);
    if (xdisplay == NULL) {
        return VENOM_ERR_PTR(VENOM_ERROR_DISPLAY_OPEN);
    }
    
    /* Allocate our display structure */
    VenomX11Display* d = VENOM_REF_NEW(VenomX11Display, x11_display_destructor);
    if (d == NULL) {
        XCloseDisplay(xdisplay);
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    /* Initialize */
    d->base.ops = &x11_display_ops;
    d->base.backend = VENOM_BACKEND_X11;
    d->xdisplay = xdisplay;
    d->default_screen = DefaultScreen(xdisplay);
    d->root_window = RootWindow(xdisplay, d->default_screen);
    
    /* Get atoms */
    d->wm_delete_window = XInternAtom(xdisplay, "WM_DELETE_WINDOW", False);
    d->wm_protocols = XInternAtom(xdisplay, "WM_PROTOCOLS", False);
    d->net_wm_name = XInternAtom(xdisplay, "_NET_WM_NAME", False);
    d->utf8_string = XInternAtom(xdisplay, "UTF8_STRING", False);
    
    /* Initialize XIM for international text input */
    if (XSetLocaleModifiers("") == NULL) {
        XSetLocaleModifiers("@im=none");  /* Fallback */
    }
    
    d->xim = XOpenIM(xdisplay, NULL, NULL, NULL);
    if (d->xim) {
        /* Get supported input styles */
        XGetIMValues(d->xim, XNQueryInputStyle, &d->xim_styles, NULL);
    } else {
        /* XIM not available - will fallback to XLookupString */
        d->xim_styles = NULL;
    }
    
    d->window_count = 0;
    d->next_window_id = 0;
    
    return VENOM_OK_PTR(d);
}
