/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_event.c - Event utilities
 */

#include "vaxp/backend/vaxp_event.h"

const char* vaxp_event_type_name(VaxpEventType type) {
    switch (type) {
        case VAXP_EVENT_NONE: return "None";
        
        case VAXP_EVENT_WINDOW_CLOSE: return "WindowClose";
        case VAXP_EVENT_WINDOW_RESIZE: return "WindowResize";
        case VAXP_EVENT_WINDOW_MOVE: return "WindowMove";
        case VAXP_EVENT_WINDOW_FOCUS_IN: return "WindowFocusIn";
        case VAXP_EVENT_WINDOW_FOCUS_OUT: return "WindowFocusOut";
        case VAXP_EVENT_WINDOW_EXPOSE: return "WindowExpose";
        case VAXP_EVENT_WINDOW_SHOW: return "WindowShow";
        case VAXP_EVENT_WINDOW_HIDE: return "WindowHide";
        
        case VAXP_EVENT_MOUSE_MOVE: return "MouseMove";
        case VAXP_EVENT_MOUSE_ENTER: return "MouseEnter";
        case VAXP_EVENT_MOUSE_LEAVE: return "MouseLeave";
        case VAXP_EVENT_MOUSE_BUTTON_DOWN: return "MouseButtonDown";
        case VAXP_EVENT_MOUSE_BUTTON_UP: return "MouseButtonUp";
        case VAXP_EVENT_MOUSE_SCROLL: return "MouseScroll";
        
        case VAXP_EVENT_KEY_DOWN: return "KeyDown";
        case VAXP_EVENT_KEY_UP: return "KeyUp";
        case VAXP_EVENT_TEXT_INPUT: return "TextInput";
        
        case VAXP_EVENT_DND_ENTER: return "DndEnter";
        case VAXP_EVENT_DND_POSITION: return "DndPosition";
        case VAXP_EVENT_DND_LEAVE: return "DndLeave";
        case VAXP_EVENT_DND_DROP: return "DndDrop";
        
        case VAXP_EVENT_QUIT: return "Quit";
        case VAXP_EVENT_USER: return "User";
        
        default: return "Unknown";
    }
}
