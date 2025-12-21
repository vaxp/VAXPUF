/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_event.h - Event types and handling
 */

#ifndef VENOM_EVENT_H
#define VENOM_EVENT_H

#include "venom/core/venom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * EVENT TYPES
 * ============================================================================ */

typedef enum VenomEventType {
    VENOM_EVENT_NONE = 0,
    
    /* Window events */
    VENOM_EVENT_WINDOW_CLOSE,
    VENOM_EVENT_WINDOW_RESIZE,
    VENOM_EVENT_WINDOW_MOVE,
    VENOM_EVENT_WINDOW_FOCUS_IN,
    VENOM_EVENT_WINDOW_FOCUS_OUT,
    VENOM_EVENT_WINDOW_EXPOSE,   /* Need to redraw */
    VENOM_EVENT_WINDOW_SHOW,
    VENOM_EVENT_WINDOW_HIDE,
    
    /* Mouse events */
    VENOM_EVENT_MOUSE_MOVE,
    VENOM_EVENT_MOUSE_ENTER,
    VENOM_EVENT_MOUSE_LEAVE,
    VENOM_EVENT_MOUSE_BUTTON_DOWN,
    VENOM_EVENT_MOUSE_BUTTON_UP,
    VENOM_EVENT_MOUSE_SCROLL,
    
    /* Keyboard events */
    VENOM_EVENT_KEY_DOWN,
    VENOM_EVENT_KEY_UP,
    VENOM_EVENT_TEXT_INPUT,
    
    /* Other */
    VENOM_EVENT_QUIT,
    VENOM_EVENT_USER,  /* Custom user event */
} VenomEventType;

/* ============================================================================
 * MOUSE BUTTONS
 * ============================================================================ */

typedef enum VenomMouseButton {
    VENOM_MOUSE_BUTTON_NONE = 0,
    VENOM_MOUSE_BUTTON_LEFT = 1,
    VENOM_MOUSE_BUTTON_MIDDLE = 2,
    VENOM_MOUSE_BUTTON_RIGHT = 3,
    VENOM_MOUSE_BUTTON_X1 = 4,
    VENOM_MOUSE_BUTTON_X2 = 5,
} VenomMouseButton;

/* ============================================================================
 * KEY MODIFIERS
 * ============================================================================ */

typedef enum VenomKeyMod {
    VENOM_KEYMOD_NONE   = 0,
    VENOM_KEYMOD_SHIFT  = (1 << 0),
    VENOM_KEYMOD_CTRL   = (1 << 1),
    VENOM_KEYMOD_ALT    = (1 << 2),
    VENOM_KEYMOD_SUPER  = (1 << 3),  /* Windows/Command key */
    VENOM_KEYMOD_CAPS   = (1 << 4),
    VENOM_KEYMOD_NUM    = (1 << 5),
} VenomKeyMod;

/* ============================================================================
 * KEY CODES (Subset, expand as needed)
 * ============================================================================ */

typedef enum VenomKeyCode {
    VENOM_KEY_UNKNOWN = 0,
    
    /* Letters */
    VENOM_KEY_A = 'a',
    VENOM_KEY_B = 'b',
    VENOM_KEY_C = 'c',
    VENOM_KEY_D = 'd',
    VENOM_KEY_E = 'e',
    VENOM_KEY_F = 'f',
    VENOM_KEY_G = 'g',
    VENOM_KEY_H = 'h',
    VENOM_KEY_I = 'i',
    VENOM_KEY_J = 'j',
    VENOM_KEY_K = 'k',
    VENOM_KEY_L = 'l',
    VENOM_KEY_M = 'm',
    VENOM_KEY_N = 'n',
    VENOM_KEY_O = 'o',
    VENOM_KEY_P = 'p',
    VENOM_KEY_Q = 'q',
    VENOM_KEY_R = 'r',
    VENOM_KEY_S = 's',
    VENOM_KEY_T = 't',
    VENOM_KEY_U = 'u',
    VENOM_KEY_V = 'v',
    VENOM_KEY_W = 'w',
    VENOM_KEY_X = 'x',
    VENOM_KEY_Y = 'y',
    VENOM_KEY_Z = 'z',
    
    /* Numbers */
    VENOM_KEY_0 = '0',
    VENOM_KEY_1 = '1',
    VENOM_KEY_2 = '2',
    VENOM_KEY_3 = '3',
    VENOM_KEY_4 = '4',
    VENOM_KEY_5 = '5',
    VENOM_KEY_6 = '6',
    VENOM_KEY_7 = '7',
    VENOM_KEY_8 = '8',
    VENOM_KEY_9 = '9',
    
    /* Special keys (using high values to avoid conflicts) */
    VENOM_KEY_ESCAPE = 256,
    VENOM_KEY_RETURN,
    VENOM_KEY_TAB,
    VENOM_KEY_BACKSPACE,
    VENOM_KEY_INSERT,
    VENOM_KEY_DELETE,
    VENOM_KEY_HOME,
    VENOM_KEY_END,
    VENOM_KEY_PAGE_UP,
    VENOM_KEY_PAGE_DOWN,
    VENOM_KEY_LEFT,
    VENOM_KEY_RIGHT,
    VENOM_KEY_UP,
    VENOM_KEY_DOWN,
    VENOM_KEY_SPACE,
    
    /* Function keys */
    VENOM_KEY_F1,
    VENOM_KEY_F2,
    VENOM_KEY_F3,
    VENOM_KEY_F4,
    VENOM_KEY_F5,
    VENOM_KEY_F6,
    VENOM_KEY_F7,
    VENOM_KEY_F8,
    VENOM_KEY_F9,
    VENOM_KEY_F10,
    VENOM_KEY_F11,
    VENOM_KEY_F12,
} VenomKeyCode;

/* ============================================================================
 * EVENT DATA STRUCTURES
 * ============================================================================ */

typedef struct VenomWindowEvent {
    VenomU32 window_id;
    VenomI32 x;
    VenomI32 y;
    VenomU32 width;
    VenomU32 height;
} VenomWindowEvent;

typedef struct VenomMouseEvent {
    VenomU32 window_id;
    VenomI32 x;           /* Position relative to window */
    VenomI32 y;
    VenomI32 root_x;      /* Position relative to screen */
    VenomI32 root_y;
    VenomMouseButton button;
    VenomKeyMod modifiers;
} VenomMouseEvent;

typedef struct VenomScrollEvent {
    VenomU32 window_id;
    VenomI32 x;
    VenomI32 y;
    VenomF32 delta_x;     /* Horizontal scroll (positive = right) */
    VenomF32 delta_y;     /* Vertical scroll (positive = down) */
    VenomKeyMod modifiers;
} VenomScrollEvent;

typedef struct VenomKeyEvent {
    VenomU32 window_id;
    VenomKeyCode key;
    VenomU32 scancode;    /* Platform-specific scan code */
    VenomKeyMod modifiers;
    VenomBool is_repeat;
} VenomKeyEvent;

typedef struct VenomTextEvent {
    VenomU32 window_id;
    char text[32];        /* UTF-8 encoded text */
} VenomTextEvent;

/* ============================================================================
 * UNIFIED EVENT STRUCTURE
 * ============================================================================ */

typedef struct VenomEvent {
    VenomEventType type;
    VenomU64 timestamp;   /* Event timestamp in milliseconds */
    
    union {
        VenomWindowEvent window;
        VenomMouseEvent mouse;
        VenomScrollEvent scroll;
        VenomKeyEvent key;
        VenomTextEvent text;
        void* user_data;  /* For VENOM_EVENT_USER */
    };
} VenomEvent;

/* ============================================================================
 * EVENT HELPERS
 * ============================================================================ */

/**
 * @brief Get name of event type for debugging
 */
const char* venom_event_type_name(VenomEventType type);

/**
 * @brief Check if event is a mouse event
 */
VENOM_INLINE VenomBool venom_event_is_mouse(VenomEventType type) {
    return type >= VENOM_EVENT_MOUSE_MOVE && type <= VENOM_EVENT_MOUSE_SCROLL;
}

/**
 * @brief Check if event is a keyboard event
 */
VENOM_INLINE VenomBool venom_event_is_keyboard(VenomEventType type) {
    return type >= VENOM_EVENT_KEY_DOWN && type <= VENOM_EVENT_TEXT_INPUT;
}

/**
 * @brief Check if event is a window event
 */
VENOM_INLINE VenomBool venom_event_is_window(VenomEventType type) {
    return type >= VENOM_EVENT_WINDOW_CLOSE && type <= VENOM_EVENT_WINDOW_HIDE;
}

#ifdef __cplusplus
}
#endif

#endif /* VENOM_EVENT_H */
