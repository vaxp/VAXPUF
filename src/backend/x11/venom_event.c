/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_event.c - Event utilities
 */

#include "venom/backend/venom_event.h"

const char* venom_event_type_name(VenomEventType type) {
    switch (type) {
        case VENOM_EVENT_NONE: return "None";
        
        case VENOM_EVENT_WINDOW_CLOSE: return "WindowClose";
        case VENOM_EVENT_WINDOW_RESIZE: return "WindowResize";
        case VENOM_EVENT_WINDOW_MOVE: return "WindowMove";
        case VENOM_EVENT_WINDOW_FOCUS_IN: return "WindowFocusIn";
        case VENOM_EVENT_WINDOW_FOCUS_OUT: return "WindowFocusOut";
        case VENOM_EVENT_WINDOW_EXPOSE: return "WindowExpose";
        case VENOM_EVENT_WINDOW_SHOW: return "WindowShow";
        case VENOM_EVENT_WINDOW_HIDE: return "WindowHide";
        
        case VENOM_EVENT_MOUSE_MOVE: return "MouseMove";
        case VENOM_EVENT_MOUSE_ENTER: return "MouseEnter";
        case VENOM_EVENT_MOUSE_LEAVE: return "MouseLeave";
        case VENOM_EVENT_MOUSE_BUTTON_DOWN: return "MouseButtonDown";
        case VENOM_EVENT_MOUSE_BUTTON_UP: return "MouseButtonUp";
        case VENOM_EVENT_MOUSE_SCROLL: return "MouseScroll";
        
        case VENOM_EVENT_KEY_DOWN: return "KeyDown";
        case VENOM_EVENT_KEY_UP: return "KeyUp";
        case VENOM_EVENT_TEXT_INPUT: return "TextInput";
        
        case VENOM_EVENT_DND_ENTER: return "DndEnter";
        case VENOM_EVENT_DND_POSITION: return "DndPosition";
        case VENOM_EVENT_DND_LEAVE: return "DndLeave";
        case VENOM_EVENT_DND_DROP: return "DndDrop";
        
        case VENOM_EVENT_QUIT: return "Quit";
        case VENOM_EVENT_USER: return "User";
        
        default: return "Unknown";
    }
}
