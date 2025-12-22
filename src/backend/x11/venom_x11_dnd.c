/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_x11_dnd.c - X11 Drag and Drop implementation (XDND Protocol v5)
 * 
 * Implements the XDND specification for drag and drop operations.
 * Reference: https://freedesktop.org/wiki/Specifications/XDND/
 */

#include "venom/backend/venom_dnd.h"
#include "venom/backend/venom_event.h"
#include "venom/core/venom_memory.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ============================================================================
 * XDND PROTOCOL CONSTANTS
 * ============================================================================ */

#define XDND_VERSION 5

/* ============================================================================
 * XDND ATOMS
 * ============================================================================ */

static struct {
    Display* display;
    VenomBool initialized;
    
    /* XDND Atoms */
    Atom XdndAware;
    Atom XdndEnter;
    Atom XdndPosition;
    Atom XdndStatus;
    Atom XdndLeave;
    Atom XdndDrop;
    Atom XdndFinished;
    Atom XdndSelection;
    Atom XdndTypeList;
    
    /* Action atoms */
    Atom XdndActionCopy;
    Atom XdndActionMove;
    Atom XdndActionLink;
    Atom XdndActionAsk;
    Atom XdndActionPrivate;
    
    /* Standard type atoms */
    Atom text_plain;
    Atom text_plain_utf8;
    Atom text_uri_list;
    Atom UTF8_STRING;
    
    /* Current drag state */
    VenomDndContext context;
    
    /* Registered windows */
    struct {
        VenomU32 id;
        Window xwindow;
    } windows[64];
    VenomU32 window_count;
    
    /* Source window for current drag */
    Window source_window;
    Window target_window;
    
    /* Available types during current drag */
    Atom* available_types;
    VenomU32 available_type_count;
    
} g_dnd = {0};

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================ */

static Window find_xwindow(VenomU32 id) {
    for (VenomU32 i = 0; i < g_dnd.window_count; i++) {
        if (g_dnd.windows[i].id == id) {
            return g_dnd.windows[i].xwindow;
        }
    }
    return None;
}

static VenomU32 find_window_id(Window xwindow) {
    for (VenomU32 i = 0; i < g_dnd.window_count; i++) {
        if (g_dnd.windows[i].xwindow == xwindow) {
            return g_dnd.windows[i].id;
        }
    }
    return 0;
}

static VenomDndAction atom_to_action(Atom action) {
    if (action == g_dnd.XdndActionCopy) return VENOM_DND_ACTION_COPY;
    if (action == g_dnd.XdndActionMove) return VENOM_DND_ACTION_MOVE;
    if (action == g_dnd.XdndActionLink) return VENOM_DND_ACTION_LINK;
    if (action == g_dnd.XdndActionAsk) return VENOM_DND_ACTION_ASK;
    if (action == g_dnd.XdndActionPrivate) return VENOM_DND_ACTION_PRIVATE;
    return VENOM_DND_ACTION_NONE;
}

static Atom action_to_atom(VenomDndAction action) {
    if (action & VENOM_DND_ACTION_COPY) return g_dnd.XdndActionCopy;
    if (action & VENOM_DND_ACTION_MOVE) return g_dnd.XdndActionMove;
    if (action & VENOM_DND_ACTION_LINK) return g_dnd.XdndActionLink;
    if (action & VENOM_DND_ACTION_ASK) return g_dnd.XdndActionAsk;
    if (action & VENOM_DND_ACTION_PRIVATE) return g_dnd.XdndActionPrivate;
    return None;
}

static const char* atom_to_mime(Atom type) {
    if (type == g_dnd.text_plain_utf8 || type == g_dnd.UTF8_STRING) {
        return VENOM_DND_MIME_TEXT_UTF8;
    }
    if (type == g_dnd.text_plain) {
        return VENOM_DND_MIME_TEXT_PLAIN;
    }
    if (type == g_dnd.text_uri_list) {
        return VENOM_DND_MIME_URI_LIST;
    }
    
    /* Get atom name for unknown types */
    char* name = XGetAtomName(g_dnd.display, type);
    if (name) {
        /* Note: This leaks memory for unknown types - should cache */
        return name;
    }
    return "application/octet-stream";
}

/* ============================================================================
 * XDND MESSAGE HANDLING
 * ============================================================================ */

/**
 * @brief Handle XdndEnter client message
 */
static VenomBool handle_xdnd_enter(XClientMessageEvent* event, VenomEvent* out) {
    Window source = (Window)event->data.l[0];
    VenomBool has_more_types = (event->data.l[1] & 1);
    int version = (event->data.l[1] >> 24) & 0xFF;
    
    if (version > XDND_VERSION) {
        /* Version too high, reject */
        return VENOM_FALSE;
    }
    
    g_dnd.source_window = source;
    g_dnd.context.state = VENOM_DND_STATE_DRAGGING;
    
    /* Free previous types */
    if (g_dnd.available_types) {
        venom_free(g_dnd.available_types, g_dnd.available_type_count * sizeof(Atom));
        g_dnd.available_types = NULL;
        g_dnd.available_type_count = 0;
    }
    
    if (has_more_types) {
        /* Get types from XdndTypeList property */
        Atom actual_type;
        int actual_format;
        unsigned long nitems, bytes_after;
        unsigned char* data = NULL;
        
        if (XGetWindowProperty(g_dnd.display, source, g_dnd.XdndTypeList,
                                0, 1024, False, XA_ATOM,
                                &actual_type, &actual_format,
                                &nitems, &bytes_after, &data) == Success) {
            if (data && nitems > 0) {
                g_dnd.available_type_count = (VenomU32)nitems;
                g_dnd.available_types = venom_alloc(nitems * sizeof(Atom));
                if (g_dnd.available_types) {
                    memcpy(g_dnd.available_types, data, nitems * sizeof(Atom));
                }
            }
            if (data) XFree(data);
        }
    } else {
        /* Types are in the message itself (up to 3) */
        VenomU32 count = 0;
        Atom types[3];
        
        for (int i = 2; i <= 4; i++) {
            if (event->data.l[i] != None) {
                types[count++] = (Atom)event->data.l[i];
            }
        }
        
        if (count > 0) {
            g_dnd.available_type_count = count;
            g_dnd.available_types = venom_alloc(count * sizeof(Atom));
            if (g_dnd.available_types) {
                memcpy(g_dnd.available_types, types, count * sizeof(Atom));
            }
        }
    }
    
    /* Build event */
    out->type = VENOM_EVENT_DND_ENTER;
    out->dnd.window_id = find_window_id(event->window);
    out->dnd.source_window = find_window_id(source);
    out->dnd.mime_type_count = g_dnd.available_type_count;
    
    /* Allocate MIME type strings */
    if (g_dnd.available_type_count > 0) {
        out->dnd.mime_types = venom_alloc(g_dnd.available_type_count * sizeof(char*));
        if (out->dnd.mime_types) {
            for (VenomU32 i = 0; i < g_dnd.available_type_count; i++) {
                char* name = XGetAtomName(g_dnd.display, g_dnd.available_types[i]);
                out->dnd.mime_types[i] = name ? name : NULL;
            }
        }
    } else {
        out->dnd.mime_types = NULL;
    }
    
    return VENOM_TRUE;
}

/**
 * @brief Handle XdndPosition client message
 */
static VenomBool handle_xdnd_position(XClientMessageEvent* event, VenomEvent* out) {
    Window source = (Window)event->data.l[0];
    int root_x = (int)(event->data.l[2] >> 16);
    int root_y = (int)(event->data.l[2] & 0xFFFF);
    Atom action = (Atom)event->data.l[4];
    
    g_dnd.source_window = source;
    g_dnd.target_window = event->window;
    g_dnd.context.x = root_x;
    g_dnd.context.y = root_y;
    
    /* Convert root coordinates to window coordinates */
    Window child;
    int win_x, win_y;
    XTranslateCoordinates(g_dnd.display, 
                          DefaultRootWindow(g_dnd.display),
                          event->window,
                          root_x, root_y,
                          &win_x, &win_y, &child);
    
    out->type = VENOM_EVENT_DND_POSITION;
    out->dnd.window_id = find_window_id(event->window);
    out->dnd.x = win_x;
    out->dnd.y = win_y;
    out->dnd.root_x = root_x;
    out->dnd.root_y = root_y;
    out->dnd.actions = (VenomU32)atom_to_action(action);
    out->timestamp = (VenomU64)event->data.l[3];
    
    return VENOM_TRUE;
}

/**
 * @brief Handle XdndLeave client message
 */
static VenomBool handle_xdnd_leave(XClientMessageEvent* event, VenomEvent* out) {
    g_dnd.source_window = None;
    
    out->type = VENOM_EVENT_DND_LEAVE;
    out->dnd.window_id = find_window_id(event->window);
    
    return VENOM_TRUE;
}

/**
 * @brief Handle XdndDrop client message
 */
static VenomBool handle_xdnd_drop(XClientMessageEvent* event, VenomEvent* out) {
    Window source = (Window)event->data.l[0];
    
    g_dnd.context.state = VENOM_DND_STATE_DROPPED;
    
    /* Convert root coordinates to window coordinates */
    Window child;
    int win_x, win_y;
    XTranslateCoordinates(g_dnd.display,
                          DefaultRootWindow(g_dnd.display),
                          event->window,
                          g_dnd.context.x, g_dnd.context.y,
                          &win_x, &win_y, &child);
    
    out->type = VENOM_EVENT_DND_DROP;
    out->dnd.window_id = find_window_id(event->window);
    out->dnd.x = win_x;
    out->dnd.y = win_y;
    out->dnd.root_x = g_dnd.context.x;
    out->dnd.root_y = g_dnd.context.y;
    out->dnd.source_window = find_window_id(source);
    out->timestamp = (VenomU64)event->data.l[2];
    
    return VENOM_TRUE;
}

/* ============================================================================
 * PUBLIC API IMPLEMENTATION
 * ============================================================================ */

VenomResult venom_dnd_init(void* display_handle) {
    if (g_dnd.initialized) {
        return VENOM_OK_UNIT();
    }
    
    Display* display = (Display*)display_handle;
    if (!display) {
        return VENOM_ERR_UNIT(VENOM_ERROR_INVALID_ARGUMENT);
    }
    
    g_dnd.display = display;
    
    /* Initialize XDND atoms */
    g_dnd.XdndAware = XInternAtom(display, "XdndAware", False);
    g_dnd.XdndEnter = XInternAtom(display, "XdndEnter", False);
    g_dnd.XdndPosition = XInternAtom(display, "XdndPosition", False);
    g_dnd.XdndStatus = XInternAtom(display, "XdndStatus", False);
    g_dnd.XdndLeave = XInternAtom(display, "XdndLeave", False);
    g_dnd.XdndDrop = XInternAtom(display, "XdndDrop", False);
    g_dnd.XdndFinished = XInternAtom(display, "XdndFinished", False);
    g_dnd.XdndSelection = XInternAtom(display, "XdndSelection", False);
    g_dnd.XdndTypeList = XInternAtom(display, "XdndTypeList", False);
    
    /* Action atoms */
    g_dnd.XdndActionCopy = XInternAtom(display, "XdndActionCopy", False);
    g_dnd.XdndActionMove = XInternAtom(display, "XdndActionMove", False);
    g_dnd.XdndActionLink = XInternAtom(display, "XdndActionLink", False);
    g_dnd.XdndActionAsk = XInternAtom(display, "XdndActionAsk", False);
    g_dnd.XdndActionPrivate = XInternAtom(display, "XdndActionPrivate", False);
    
    /* Type atoms */
    g_dnd.text_plain = XInternAtom(display, "text/plain", False);
    g_dnd.text_plain_utf8 = XInternAtom(display, "text/plain;charset=utf-8", False);
    g_dnd.text_uri_list = XInternAtom(display, "text/uri-list", False);
    g_dnd.UTF8_STRING = XInternAtom(display, "UTF8_STRING", False);
    
    g_dnd.context.state = VENOM_DND_STATE_IDLE;
    g_dnd.initialized = VENOM_TRUE;
    
    return VENOM_OK_UNIT();
}

void venom_dnd_shutdown(void) {
    if (!g_dnd.initialized) return;
    
    /* Free available types */
    if (g_dnd.available_types) {
        venom_free(g_dnd.available_types, g_dnd.available_type_count * sizeof(Atom));
        g_dnd.available_types = NULL;
    }
    
    /* Clear state */
    memset(&g_dnd.context, 0, sizeof(g_dnd.context));
    g_dnd.window_count = 0;
    g_dnd.initialized = VENOM_FALSE;
}

VenomResult venom_dnd_register_window(VenomU32 window_id, void* native_window) {
    if (!g_dnd.initialized) {
        return VENOM_ERR_UNIT(VENOM_ERROR_NOT_INITIALIZED);
    }
    
    if (g_dnd.window_count >= 64) {
        return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    Window xwindow = (Window)(uintptr_t)native_window;
    
    /* Set XdndAware property */
    Atom version = XDND_VERSION;
    XChangeProperty(g_dnd.display, xwindow, g_dnd.XdndAware,
                    XA_ATOM, 32, PropModeReplace,
                    (unsigned char*)&version, 1);
    
    /* Register window */
    g_dnd.windows[g_dnd.window_count].id = window_id;
    g_dnd.windows[g_dnd.window_count].xwindow = xwindow;
    g_dnd.window_count++;
    
    return VENOM_OK_UNIT();
}

void venom_dnd_unregister_window(VenomU32 window_id) {
    for (VenomU32 i = 0; i < g_dnd.window_count; i++) {
        if (g_dnd.windows[i].id == window_id) {
            /* Remove XdndAware property */
            XDeleteProperty(g_dnd.display, g_dnd.windows[i].xwindow, g_dnd.XdndAware);
            
            /* Shift remaining windows */
            for (VenomU32 j = i; j < g_dnd.window_count - 1; j++) {
                g_dnd.windows[j] = g_dnd.windows[j + 1];
            }
            g_dnd.window_count--;
            break;
        }
    }
}

VenomBool venom_dnd_is_dragging(void) {
    return g_dnd.context.state != VENOM_DND_STATE_IDLE;
}

const VenomDndContext* venom_dnd_get_context(void) {
    return &g_dnd.context;
}

VenomResult venom_dnd_accept(VenomU32 window_id, VenomI32 x, VenomI32 y,
                              VenomDndAction action) {
    if (!g_dnd.initialized || g_dnd.source_window == None) {
        return VENOM_ERR_UNIT(VENOM_ERROR_INVALID_STATE);
    }
    
    Window xwindow = find_xwindow(window_id);
    if (xwindow == None) {
        return VENOM_ERR_UNIT(VENOM_ERROR_INVALID_ARGUMENT);
    }
    
    /* Send XdndStatus message to source */
    XClientMessageEvent status = {0};
    status.type = ClientMessage;
    status.display = g_dnd.display;
    status.window = g_dnd.source_window;
    status.message_type = g_dnd.XdndStatus;
    status.format = 32;
    status.data.l[0] = xwindow;                           /* Target window */
    status.data.l[1] = 1 | (1 << 1);                      /* Accept drop, want position updates */
    status.data.l[2] = 0;                                  /* Rectangle (not used) */
    status.data.l[3] = 0;
    status.data.l[4] = action_to_atom(action);            /* Accepted action */
    
    XSendEvent(g_dnd.display, g_dnd.source_window, False, NoEventMask, (XEvent*)&status);
    XFlush(g_dnd.display);
    
    g_dnd.context.selected_action = action;
    
    return VENOM_OK_UNIT();
}

void venom_dnd_reject(VenomU32 window_id) {
    if (!g_dnd.initialized || g_dnd.source_window == None) {
        return;
    }
    
    Window xwindow = find_xwindow(window_id);
    if (xwindow == None) return;
    
    /* Send XdndStatus with reject */
    XClientMessageEvent status = {0};
    status.type = ClientMessage;
    status.display = g_dnd.display;
    status.window = g_dnd.source_window;
    status.message_type = g_dnd.XdndStatus;
    status.format = 32;
    status.data.l[0] = xwindow;
    status.data.l[1] = 0;  /* Reject */
    status.data.l[2] = 0;
    status.data.l[3] = 0;
    status.data.l[4] = None;
    
    XSendEvent(g_dnd.display, g_dnd.source_window, False, NoEventMask, (XEvent*)&status);
    XFlush(g_dnd.display);
}

VenomResult venom_dnd_get_drop_data(VenomU32 window_id,
                                     const char* mime_type,
                                     VenomDndData* out_data) {
    if (!g_dnd.initialized || g_dnd.context.state != VENOM_DND_STATE_DROPPED) {
        return VENOM_ERR_UNIT(VENOM_ERROR_INVALID_STATE);
    }
    
    Window xwindow = find_xwindow(window_id);
    if (xwindow == None || !out_data) {
        return VENOM_ERR_UNIT(VENOM_ERROR_INVALID_ARGUMENT);
    }
    
    /* Find matching atom */
    Atom target_type = None;
    for (VenomU32 i = 0; i < g_dnd.available_type_count; i++) {
        const char* type_name = atom_to_mime(g_dnd.available_types[i]);
        if (strcmp(type_name, mime_type) == 0) {
            target_type = g_dnd.available_types[i];
            break;
        }
    }
    
    if (target_type == None) {
        /* Try UTF8_STRING as fallback for text */
        if (strstr(mime_type, "text") != NULL) {
            target_type = g_dnd.UTF8_STRING;
        } else {
            return VENOM_ERR_UNIT(VENOM_ERROR_NOT_FOUND);
        }
    }
    
    /* Request selection conversion */
    Atom property = XInternAtom(g_dnd.display, "VENOM_DND_DATA", False);
    XConvertSelection(g_dnd.display, g_dnd.XdndSelection, target_type,
                      property, xwindow, CurrentTime);
    XFlush(g_dnd.display);
    
    /* Wait for SelectionNotify event */
    XEvent event;
    for (int attempts = 0; attempts < 100; attempts++) {
        if (XCheckTypedWindowEvent(g_dnd.display, xwindow, SelectionNotify, &event)) {
            if (event.xselection.property != None) {
                /* Read the data */
                Atom actual_type;
                int actual_format;
                unsigned long nitems, bytes_after;
                unsigned char* data = NULL;
                
                if (XGetWindowProperty(g_dnd.display, xwindow, property,
                                        0, 1024 * 1024, True, AnyPropertyType,
                                        &actual_type, &actual_format,
                                        &nitems, &bytes_after, &data) == Success) {
                    if (data && nitems > 0) {
                        /* Copy data */
                        VenomSize data_size = nitems * (actual_format / 8);
                        out_data->data = venom_alloc(data_size + 1);
                        if (out_data->data) {
                            memcpy(out_data->data, data, data_size);
                            ((char*)out_data->data)[data_size] = '\0';
                            out_data->size = data_size;
                            strncpy(out_data->mime_type, mime_type, 
                                    sizeof(out_data->mime_type) - 1);
                            out_data->mime_type[sizeof(out_data->mime_type) - 1] = '\0';
                        }
                        XFree(data);
                        return VENOM_OK_UNIT();
                    }
                    if (data) XFree(data);
                }
            }
            return VENOM_ERR_UNIT(VENOM_ERROR_IO);
        }
        
        /* Wait a bit */
        struct timespec ts = {0, 10000000}; /* 10ms */
        nanosleep(&ts, NULL);
    }
    
    return VENOM_ERR_UNIT(VENOM_ERROR_TIMEOUT);
}

void venom_dnd_finish_drop(VenomU32 window_id, VenomBool success) {
    if (!g_dnd.initialized || g_dnd.source_window == None) {
        return;
    }
    
    Window xwindow = find_xwindow(window_id);
    if (xwindow == None) return;
    
    /* Send XdndFinished */
    XClientMessageEvent finished = {0};
    finished.type = ClientMessage;
    finished.display = g_dnd.display;
    finished.window = g_dnd.source_window;
    finished.message_type = g_dnd.XdndFinished;
    finished.format = 32;
    finished.data.l[0] = xwindow;
    finished.data.l[1] = success ? 1 : 0;
    finished.data.l[2] = success ? action_to_atom(g_dnd.context.selected_action) : None;
    
    XSendEvent(g_dnd.display, g_dnd.source_window, False, NoEventMask, (XEvent*)&finished);
    XFlush(g_dnd.display);
    
    /* Reset state */
    g_dnd.context.state = VENOM_DND_STATE_IDLE;
    g_dnd.source_window = None;
    g_dnd.target_window = None;
}

/* ============================================================================
 * DND DATA LIST HELPERS
 * ============================================================================ */

VenomDndDataList* venom_dnd_data_list_create(void) {
    VenomDndDataList* list = venom_alloc(sizeof(VenomDndDataList));
    if (list) {
        memset(list, 0, sizeof(VenomDndDataList));
    }
    return list;
}

void venom_dnd_data_list_destroy(VenomDndDataList* list) {
    if (!list) return;
    
    for (VenomU32 i = 0; i < list->count; i++) {
        if (list->items[i].data) {
            venom_free(list->items[i].data, list->items[i].size);
        }
    }
    
    if (list->items) {
        venom_free(list->items, list->capacity * sizeof(VenomDndData));
    }
    
    venom_free(list, sizeof(VenomDndDataList));
}

static VenomResult dnd_data_list_grow(VenomDndDataList* list) {
    VenomU32 new_capacity = list->capacity == 0 ? 4 : list->capacity * 2;
    VenomDndData* new_items = venom_alloc(new_capacity * sizeof(VenomDndData));
    if (!new_items) {
        return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    if (list->items) {
        memcpy(new_items, list->items, list->count * sizeof(VenomDndData));
        venom_free(list->items, list->capacity * sizeof(VenomDndData));
    }
    
    list->items = new_items;
    list->capacity = new_capacity;
    return VENOM_OK_UNIT();
}

VenomResult venom_dnd_data_list_add_text(VenomDndDataList* list, const char* text) {
    if (!list || !text) {
        return VENOM_ERR_UNIT(VENOM_ERROR_INVALID_ARGUMENT);
    }
    
    return venom_dnd_data_list_add(list, VENOM_DND_MIME_TEXT_UTF8, 
                                    text, strlen(text));
}

VenomResult venom_dnd_data_list_add_uris(VenomDndDataList* list, const char** uris) {
    if (!list || !uris) {
        return VENOM_ERR_UNIT(VENOM_ERROR_INVALID_ARGUMENT);
    }
    
    /* Calculate total size */
    VenomSize total_size = 0;
    VenomU32 count = 0;
    while (uris[count]) {
        total_size += strlen(uris[count]) + 2;  /* +2 for \r\n */
        count++;
    }
    
    if (count == 0) {
        return VENOM_OK_UNIT();
    }
    
    /* Build URI list string */
    char* uri_list = venom_alloc(total_size + 1);
    if (!uri_list) {
        return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    char* ptr = uri_list;
    for (VenomU32 i = 0; i < count; i++) {
        size_t len = strlen(uris[i]);
        memcpy(ptr, uris[i], len);
        ptr += len;
        *ptr++ = '\r';
        *ptr++ = '\n';
    }
    *ptr = '\0';
    
    VenomResult result = venom_dnd_data_list_add(list, VENOM_DND_MIME_URI_LIST,
                                                  uri_list, total_size);
    venom_free(uri_list, total_size + 1);
    return result;
}

VenomResult venom_dnd_data_list_add(VenomDndDataList* list,
                                     const char* mime_type,
                                     const void* data,
                                     VenomSize size) {
    if (!list || !mime_type || !data || size == 0) {
        return VENOM_ERR_UNIT(VENOM_ERROR_INVALID_ARGUMENT);
    }
    
    if (list->count >= list->capacity) {
        VenomResult result = dnd_data_list_grow(list);
        if (!result.ok) return result;
    }
    
    VenomDndData* item = &list->items[list->count];
    strncpy(item->mime_type, mime_type, sizeof(item->mime_type) - 1);
    item->mime_type[sizeof(item->mime_type) - 1] = '\0';
    
    item->data = venom_alloc(size);
    if (!item->data) {
        return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    memcpy(item->data, data, size);
    item->size = size;
    list->count++;
    
    return VENOM_OK_UNIT();
}

const VenomDndData* venom_dnd_data_list_find(const VenomDndDataList* list,
                                              const char* mime_type) {
    if (!list || !mime_type) return NULL;
    
    for (VenomU32 i = 0; i < list->count; i++) {
        if (strcmp(list->items[i].mime_type, mime_type) == 0) {
            return &list->items[i];
        }
    }
    return NULL;
}

VenomResult venom_dnd_parse_uri_list(const VenomDndData* data,
                                      char*** out_paths,
                                      VenomU32* out_count) {
    if (!data || !out_paths || !out_count || !data->data) {
        return VENOM_ERR_UNIT(VENOM_ERROR_INVALID_ARGUMENT);
    }
    
    const char* text = (const char*)data->data;
    
    /* Count lines */
    VenomU32 count = 0;
    const char* p = text;
    while (*p) {
        if (*p == '\n' || *p == '\r') {
            if (p > text && *(p-1) != '\n' && *(p-1) != '\r') {
                count++;
            }
        }
        p++;
    }
    if (p > text && *(p-1) != '\n' && *(p-1) != '\r') {
        count++;  /* Last line without newline */
    }
    
    if (count == 0) {
        *out_paths = NULL;
        *out_count = 0;
        return VENOM_OK_UNIT();
    }
    
    /* Allocate path array */
    char** paths = venom_alloc((count + 1) * sizeof(char*));
    if (!paths) {
        return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    /* Parse paths */
    VenomU32 idx = 0;
    const char* line_start = text;
    p = text;
    
    while (*p && idx < count) {
        if (*p == '\n' || *p == '\r' || *(p+1) == '\0') {
            const char* line_end = (*p == '\n' || *p == '\r') ? p : p + 1;
            size_t line_len = line_end - line_start;
            
            if (line_len > 0) {
                /* Skip "file://" prefix */
                const char* path_start = line_start;
                if (line_len > 7 && strncmp(line_start, "file://", 7) == 0) {
                    path_start += 7;
                    line_len -= 7;
                }
                
                /* Allocate and copy path */
                paths[idx] = venom_alloc(line_len + 1);
                if (paths[idx]) {
                    memcpy(paths[idx], path_start, line_len);
                    paths[idx][line_len] = '\0';
                    idx++;
                }
            }
            
            /* Skip to next line */
            while (*p == '\n' || *p == '\r') p++;
            line_start = p;
        } else {
            p++;
        }
    }
    
    paths[idx] = NULL;
    *out_paths = paths;
    *out_count = idx;
    
    return VENOM_OK_UNIT();
}

/* ============================================================================
 * EVENT TRANSLATION (Called from display event loop)
 * ============================================================================ */

/**
 * @brief Process XClientMessageEvent for DND
 * 
 * Called from the main event loop to check if a client message is DND-related.
 * 
 * @param event X client message event
 * @param out Output VENOMUI event
 * @return VENOM_TRUE if this was a DND event
 */
VenomBool venom_dnd_process_client_message(void* xevent, VenomEvent* out) {
    if (!g_dnd.initialized) return VENOM_FALSE;
    
    XClientMessageEvent* event = (XClientMessageEvent*)xevent;
    
    if (event->message_type == g_dnd.XdndEnter) {
        return handle_xdnd_enter(event, out);
    }
    if (event->message_type == g_dnd.XdndPosition) {
        return handle_xdnd_position(event, out);
    }
    if (event->message_type == g_dnd.XdndLeave) {
        return handle_xdnd_leave(event, out);
    }
    if (event->message_type == g_dnd.XdndDrop) {
        return handle_xdnd_drop(event, out);
    }
    
    return VENOM_FALSE;
}
