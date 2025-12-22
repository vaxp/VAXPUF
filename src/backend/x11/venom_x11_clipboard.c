/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_x11_clipboard.c - X11 Clipboard implementation
 * 
 * Implements X11 Selections protocol (ICCCM) for clipboard operations.
 * Supports both CLIPBOARD and PRIMARY selections with UTF-8 text.
 */

#include "venom/backend/venom_clipboard.h"
#include "venom/core/venom_memory.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* ============================================================================
 * CLIPBOARD STATE
 * ============================================================================ */

static struct {
    Display* display;
    Window window;
    VenomBool initialized;
    
    /* Selection atoms */
    Atom CLIPBOARD;
    Atom PRIMARY; 
    Atom TARGETS;
    Atom UTF8_STRING;
    Atom TEXT;
    Atom STRING;
    Atom VENOM_CLIPBOARD_DATA;
    Atom INCR;
    
    /* Currently owned selection content */
    struct {
        char* data;
        VenomSize size;
        VenomBool owned;
    } clipboard_content;
    
    struct {
        char* data;
        VenomSize size;
        VenomBool owned;
    } primary_content;
    
} g_clipboard = {0};

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================ */

static Atom selection_to_atom(VenomClipboardSelection sel) {
    return (sel == VENOM_CLIPBOARD_PRIMARY) ? g_clipboard.PRIMARY : g_clipboard.CLIPBOARD;
}

static void* get_content_for_selection(VenomClipboardSelection sel) {
    if (sel == VENOM_CLIPBOARD_PRIMARY) {
        return &g_clipboard.primary_content;
    }
    return &g_clipboard.clipboard_content;
}

static void free_content(void* content_ptr) {
    struct {
        char* data;
        VenomSize size;
        VenomBool owned;
    }* content = content_ptr;
    
    if (content->data) {
        venom_free(content->data, content->size + 1);
        content->data = NULL;
        content->size = 0;
    }
    content->owned = VENOM_FALSE;
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VenomResult venom_clipboard_init(void* display_handle, void* window_handle) {
    if (g_clipboard.initialized) {
        return VENOM_OK_UNIT();
    }
    
    Display* display = (Display*)display_handle;
    Window window = (Window)(uintptr_t)window_handle;
    
    if (!display || window == None) {
        return VENOM_ERR_UNIT(VENOM_ERROR_INVALID_ARGUMENT);
    }
    
    g_clipboard.display = display;
    g_clipboard.window = window;
    
    /* Initialize atoms */
    g_clipboard.CLIPBOARD = XInternAtom(display, "CLIPBOARD", False);
    g_clipboard.PRIMARY = XA_PRIMARY;
    g_clipboard.TARGETS = XInternAtom(display, "TARGETS", False);
    g_clipboard.UTF8_STRING = XInternAtom(display, "UTF8_STRING", False);
    g_clipboard.TEXT = XInternAtom(display, "TEXT", False);
    g_clipboard.STRING = XA_STRING;
    g_clipboard.VENOM_CLIPBOARD_DATA = XInternAtom(display, "VENOM_CLIPBOARD_DATA", False);
    g_clipboard.INCR = XInternAtom(display, "INCR", False);
    
    g_clipboard.initialized = VENOM_TRUE;
    
    return VENOM_OK_UNIT();
}

void venom_clipboard_shutdown(void) {
    if (!g_clipboard.initialized) return;
    
    free_content(&g_clipboard.clipboard_content);
    free_content(&g_clipboard.primary_content);
    
    g_clipboard.initialized = VENOM_FALSE;
}

VenomResult venom_clipboard_set_text(VenomClipboardSelection selection,
                                      const char* text) {
    if (!g_clipboard.initialized) {
        return VENOM_ERR_UNIT(VENOM_ERROR_NOT_INITIALIZED);
    }
    
    if (!text) {
        return VENOM_ERR_UNIT(VENOM_ERROR_INVALID_ARGUMENT);
    }
    
    Atom sel_atom = selection_to_atom(selection);
    void* content_ptr = get_content_for_selection(selection);
    
    /* Cast to access struct fields */
    struct {
        char* data;
        VenomSize size;
        VenomBool owned;
    }* content = content_ptr;
    
    /* Free existing content */
    free_content(content_ptr);
    
    /* Copy new content */
    VenomSize len = strlen(text);
    content->data = venom_alloc(len + 1);
    if (!content->data) {
        return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    memcpy(content->data, text, len + 1);
    content->size = len;
    
    /* Take ownership of the selection */
    XSetSelectionOwner(g_clipboard.display, sel_atom, g_clipboard.window, CurrentTime);
    XFlush(g_clipboard.display);
    
    /* Verify we got ownership */
    Window owner = XGetSelectionOwner(g_clipboard.display, sel_atom);
    if (owner != g_clipboard.window) {
        free_content(content_ptr);
        return VENOM_ERR_UNIT(VENOM_ERROR_IO);
    }
    
    content->owned = VENOM_TRUE;
    
    return VENOM_OK_UNIT();
}

VenomResult venom_clipboard_get_text(VenomClipboardSelection selection,
                                      char** out_text) {
    if (!g_clipboard.initialized) {
        return VENOM_ERR_UNIT(VENOM_ERROR_NOT_INITIALIZED);
    }
    
    if (!out_text) {
        return VENOM_ERR_UNIT(VENOM_ERROR_INVALID_ARGUMENT);
    }
    
    *out_text = NULL;
    
    Atom sel_atom = selection_to_atom(selection);
    
    /* Check if we own this selection */
    void* content_ptr = get_content_for_selection(selection);
    struct {
        char* data;
        VenomSize size;
        VenomBool owned;
    }* content = content_ptr;
    
    if (content->owned && content->data) {
        /* We own it, return a copy */
        char* copy = venom_alloc(content->size + 1);
        if (!copy) {
            return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
        }
        memcpy(copy, content->data, content->size + 1);
        *out_text = copy;
        return VENOM_OK_UNIT();
    }
    
    /* Request selection from owner */
    Window owner = XGetSelectionOwner(g_clipboard.display, sel_atom);
    if (owner == None) {
        return VENOM_ERR_UNIT(VENOM_ERROR_NOT_FOUND);
    }
    
    /* Request conversion to UTF8_STRING */
    XConvertSelection(g_clipboard.display, sel_atom, g_clipboard.UTF8_STRING,
                      g_clipboard.VENOM_CLIPBOARD_DATA, g_clipboard.window, CurrentTime);
    XFlush(g_clipboard.display);
    
    /* Wait for SelectionNotify event */
    XEvent event;
    struct timespec start_time, current_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    for (;;) {
        /* Check timeout (2 seconds) */
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        double elapsed = (current_time.tv_sec - start_time.tv_sec) +
                         (current_time.tv_nsec - start_time.tv_nsec) / 1e9;
        if (elapsed > 2.0) {
            return VENOM_ERR_UNIT(VENOM_ERROR_TIMEOUT);
        }
        
        if (XCheckTypedWindowEvent(g_clipboard.display, g_clipboard.window, 
                                    SelectionNotify, &event)) {
            if (event.xselection.selection == sel_atom) {
                if (event.xselection.property == None) {
                    /* Conversion failed, try STRING as fallback */
                    XConvertSelection(g_clipboard.display, sel_atom, g_clipboard.STRING,
                                      g_clipboard.VENOM_CLIPBOARD_DATA, g_clipboard.window, CurrentTime);
                    XFlush(g_clipboard.display);
                    continue;
                }
                
                /* Read the data */
                Atom actual_type;
                int actual_format;
                unsigned long nitems, bytes_after;
                unsigned char* data = NULL;
                
                if (XGetWindowProperty(g_clipboard.display, g_clipboard.window,
                                        g_clipboard.VENOM_CLIPBOARD_DATA,
                                        0, 1024 * 1024, True, AnyPropertyType,
                                        &actual_type, &actual_format,
                                        &nitems, &bytes_after, &data) == Success) {
                    if (data && nitems > 0) {
                        /* Check for INCR (incremental transfer) */
                        if (actual_type == g_clipboard.INCR) {
                            /* Not implemented - would need to receive in chunks */
                            XFree(data);
                            return VENOM_ERR_UNIT(VENOM_ERROR_NOT_SUPPORTED);
                        }
                        
                        /* Copy the text */
                        VenomSize text_size = nitems * (actual_format / 8);
                        char* text = venom_alloc(text_size + 1);
                        if (!text) {
                            XFree(data);
                            return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
                        }
                        
                        memcpy(text, data, text_size);
                        text[text_size] = '\0';
                        XFree(data);
                        
                        *out_text = text;
                        return VENOM_OK_UNIT();
                    }
                    if (data) XFree(data);
                }
                
                return VENOM_ERR_UNIT(VENOM_ERROR_NOT_FOUND);
            }
        }
        
        /* Small delay to prevent busy waiting */
        struct timespec delay = {0, 1000000}; /* 1ms */
        nanosleep(&delay, NULL);
    }
}

VenomBool venom_clipboard_has_text(VenomClipboardSelection selection) {
    if (!g_clipboard.initialized) return VENOM_FALSE;
    
    Atom sel_atom = selection_to_atom(selection);
    
    /* Check if we own it */
    void* content_ptr = get_content_for_selection(selection);
    struct {
        char* data;
        VenomSize size;
        VenomBool owned;
    }* content = content_ptr;
    
    if (content->owned && content->data) {
        return VENOM_TRUE;
    }
    
    /* Check if there's an owner */
    Window owner = XGetSelectionOwner(g_clipboard.display, sel_atom);
    return owner != None;
}

void venom_clipboard_clear(VenomClipboardSelection selection) {
    if (!g_clipboard.initialized) return;
    
    void* content_ptr = get_content_for_selection(selection);
    struct {
        char* data;
        VenomSize size;
        VenomBool owned;
    }* content = content_ptr;
    
    if (content->owned) {
        Atom sel_atom = selection_to_atom(selection);
        XSetSelectionOwner(g_clipboard.display, sel_atom, None, CurrentTime);
        XFlush(g_clipboard.display);
        free_content(content_ptr);
    }
}

/* ============================================================================
 * EVENT HANDLING
 * ============================================================================ */

VenomBool venom_clipboard_process_event(void* xevent) {
    if (!g_clipboard.initialized) return VENOM_FALSE;
    
    XEvent* event = (XEvent*)xevent;
    
    switch (event->type) {
        case SelectionRequest: {
            XSelectionRequestEvent* req = &event->xselectionrequest;
            XSelectionEvent response = {0};
            
            response.type = SelectionNotify;
            response.display = g_clipboard.display;
            response.requestor = req->requestor;
            response.selection = req->selection;
            response.target = req->target;
            response.property = None;
            response.time = req->time;
            
            /* Determine which content to use */
            void* content_ptr = NULL;
            if (req->selection == g_clipboard.CLIPBOARD) {
                content_ptr = &g_clipboard.clipboard_content;
            } else if (req->selection == g_clipboard.PRIMARY) {
                content_ptr = &g_clipboard.primary_content;
            }
            
            if (content_ptr) {
                struct {
                    char* data;
                    VenomSize size;
                    VenomBool owned;
                }* content = content_ptr;
                
                if (content->owned && content->data) {
                    if (req->target == g_clipboard.TARGETS) {
                        /* Return list of supported targets */
                        Atom targets[] = {
                            g_clipboard.TARGETS,
                            g_clipboard.UTF8_STRING,
                            g_clipboard.STRING,
                            g_clipboard.TEXT
                        };
                        
                        XChangeProperty(g_clipboard.display, req->requestor,
                                        req->property, XA_ATOM, 32, PropModeReplace,
                                        (unsigned char*)targets, 
                                        sizeof(targets) / sizeof(Atom));
                        response.property = req->property;
                    }
                    else if (req->target == g_clipboard.UTF8_STRING ||
                             req->target == g_clipboard.STRING ||
                             req->target == g_clipboard.TEXT) {
                        /* Return the text data */
                        XChangeProperty(g_clipboard.display, req->requestor,
                                        req->property, req->target, 8, PropModeReplace,
                                        (unsigned char*)content->data, content->size);
                        response.property = req->property;
                    }
                }
            }
            
            XSendEvent(g_clipboard.display, req->requestor, False, 0, (XEvent*)&response);
            XFlush(g_clipboard.display);
            
            return VENOM_TRUE;
        }
        
        case SelectionClear: {
            XSelectionClearEvent* clear = &event->xselectionclear;
            
            if (clear->selection == g_clipboard.CLIPBOARD) {
                free_content(&g_clipboard.clipboard_content);
            } else if (clear->selection == g_clipboard.PRIMARY) {
                free_content(&g_clipboard.primary_content);
            }
            
            return VENOM_TRUE;
        }
        
        default:
            break;
    }
    
    return VENOM_FALSE;
}

void venom_clipboard_on_window_destroy(void) {
    if (!g_clipboard.initialized) return;
    
    /* If we own any selections, we should handle them properly */
    /* For now, just clear our ownership */
    if (g_clipboard.clipboard_content.owned) {
        XSetSelectionOwner(g_clipboard.display, g_clipboard.CLIPBOARD, None, CurrentTime);
    }
    if (g_clipboard.primary_content.owned) {
        XSetSelectionOwner(g_clipboard.display, g_clipboard.PRIMARY, None, CurrentTime);
    }
    
    XFlush(g_clipboard.display);
    
    free_content(&g_clipboard.clipboard_content);
    free_content(&g_clipboard.primary_content);
}
