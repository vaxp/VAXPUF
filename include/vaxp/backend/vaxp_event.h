/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_event.h - Event types and handling
 */

#ifndef VAXP_EVENT_H
#define VAXP_EVENT_H

#include "vaxp/core/vaxp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * EVENT TYPES
 * ============================================================================ */

typedef enum VaxpEventType {
    VAXP_EVENT_NONE = 0,
    
    /* Window events */
    VAXP_EVENT_WINDOW_CLOSE,
    VAXP_EVENT_WINDOW_RESIZE,
    VAXP_EVENT_WINDOW_MOVE,
    VAXP_EVENT_WINDOW_FOCUS_IN,
    VAXP_EVENT_WINDOW_FOCUS_OUT,
    VAXP_EVENT_WINDOW_EXPOSE,   /* Need to redraw */
    VAXP_EVENT_WINDOW_SHOW,
    VAXP_EVENT_WINDOW_HIDE,
    
    /* Mouse events */
    VAXP_EVENT_MOUSE_MOVE,
    VAXP_EVENT_MOUSE_ENTER,
    VAXP_EVENT_MOUSE_LEAVE,
    VAXP_EVENT_MOUSE_BUTTON_DOWN,
    VAXP_EVENT_MOUSE_BUTTON_UP,
    VAXP_EVENT_MOUSE_SCROLL,
    
    /* Keyboard events */
    VAXP_EVENT_KEY_DOWN,
    VAXP_EVENT_KEY_UP,
    VAXP_EVENT_TEXT_INPUT,
    
    /* Drag and Drop events */
    VAXP_EVENT_DND_ENTER,      /* Drag entered window */
    VAXP_EVENT_DND_POSITION,   /* Drag moved within window */
    VAXP_EVENT_DND_LEAVE,      /* Drag left window */
    VAXP_EVENT_DND_DROP,       /* Drop occurred */
    
    /* Other */
    VAXP_EVENT_QUIT,
    VAXP_EVENT_USER,  /* Custom user event */
} VaxpEventType;

/* ============================================================================
 * MOUSE BUTTONS
 * ============================================================================ */

typedef enum VaxpMouseButton {
    VAXP_MOUSE_BUTTON_NONE = 0,
    VAXP_MOUSE_BUTTON_LEFT = 1,
    VAXP_MOUSE_BUTTON_MIDDLE = 2,
    VAXP_MOUSE_BUTTON_RIGHT = 3,
    VAXP_MOUSE_BUTTON_X1 = 4,
    VAXP_MOUSE_BUTTON_X2 = 5,
} VaxpMouseButton;

/* ============================================================================
 * KEY MODIFIERS
 * ============================================================================ */

typedef enum VaxpKeyMod {
    VAXP_KEYMOD_NONE   = 0,
    VAXP_KEYMOD_SHIFT  = (1 << 0),
    VAXP_KEYMOD_CTRL   = (1 << 1),
    VAXP_KEYMOD_ALT    = (1 << 2),
    VAXP_KEYMOD_SUPER  = (1 << 3),  /* Windows/Command key */
    VAXP_KEYMOD_CAPS   = (1 << 4),
    VAXP_KEYMOD_NUM    = (1 << 5),
} VaxpKeyMod;

/* ============================================================================
 * KEY CODES (Subset, expand as needed)
 * ============================================================================ */

typedef enum VaxpKeyCode {
    VAXP_KEY_UNKNOWN = 0,
    
    /* Letters */
    VAXP_KEY_A = 'a',
    VAXP_KEY_B = 'b',
    VAXP_KEY_C = 'c',
    VAXP_KEY_D = 'd',
    VAXP_KEY_E = 'e',
    VAXP_KEY_F = 'f',
    VAXP_KEY_G = 'g',
    VAXP_KEY_H = 'h',
    VAXP_KEY_I = 'i',
    VAXP_KEY_J = 'j',
    VAXP_KEY_K = 'k',
    VAXP_KEY_L = 'l',
    VAXP_KEY_M = 'm',
    VAXP_KEY_N = 'n',
    VAXP_KEY_O = 'o',
    VAXP_KEY_P = 'p',
    VAXP_KEY_Q = 'q',
    VAXP_KEY_R = 'r',
    VAXP_KEY_S = 's',
    VAXP_KEY_T = 't',
    VAXP_KEY_U = 'u',
    VAXP_KEY_V = 'v',
    VAXP_KEY_W = 'w',
    VAXP_KEY_X = 'x',
    VAXP_KEY_Y = 'y',
    VAXP_KEY_Z = 'z',
    
    /* Numbers */
    VAXP_KEY_0 = '0',
    VAXP_KEY_1 = '1',
    VAXP_KEY_2 = '2',
    VAXP_KEY_3 = '3',
    VAXP_KEY_4 = '4',
    VAXP_KEY_5 = '5',
    VAXP_KEY_6 = '6',
    VAXP_KEY_7 = '7',
    VAXP_KEY_8 = '8',
    VAXP_KEY_9 = '9',
    
    /* Special keys (using high values to avoid conflicts) */
    VAXP_KEY_ESCAPE = 256,
    VAXP_KEY_RETURN,
    VAXP_KEY_TAB,
    VAXP_KEY_BACKSPACE,
    VAXP_KEY_INSERT,
    VAXP_KEY_DELETE,
    VAXP_KEY_HOME,
    VAXP_KEY_END,
    VAXP_KEY_PAGE_UP,
    VAXP_KEY_PAGE_DOWN,
    VAXP_KEY_LEFT,
    VAXP_KEY_RIGHT,
    VAXP_KEY_UP,
    VAXP_KEY_DOWN,
    VAXP_KEY_SPACE,
    
    /* Function keys */
    VAXP_KEY_F1,
    VAXP_KEY_F2,
    VAXP_KEY_F3,
    VAXP_KEY_F4,
    VAXP_KEY_F5,
    VAXP_KEY_F6,
    VAXP_KEY_F7,
    VAXP_KEY_F8,
    VAXP_KEY_F9,
    VAXP_KEY_F10,
    VAXP_KEY_F11,
    VAXP_KEY_F12,
} VaxpKeyCode;

/* ============================================================================
 * EVENT DATA STRUCTURES
 * ============================================================================ */

typedef struct VaxpWindowEvent {
    VaxpU32 window_id;
    VaxpI32 x;
    VaxpI32 y;
    VaxpU32 width;
    VaxpU32 height;
} VaxpWindowEvent;

typedef struct VaxpMouseEvent {
    VaxpU32 window_id;
    VaxpI32 x;           /* Position relative to window */
    VaxpI32 y;
    VaxpI32 root_x;      /* Position relative to screen */
    VaxpI32 root_y;
    VaxpMouseButton button;
    VaxpKeyMod modifiers;
} VaxpMouseEvent;

typedef struct VaxpScrollEvent {
    VaxpU32 window_id;
    VaxpI32 x;
    VaxpI32 y;
    VaxpF32 delta_x;     /* Horizontal scroll (positive = right) */
    VaxpF32 delta_y;     /* Vertical scroll (positive = down) */
    VaxpKeyMod modifiers;
} VaxpScrollEvent;

typedef struct VaxpKeyEvent {
    VaxpU32 window_id;
    VaxpKeyCode key;
    VaxpU32 scancode;    /* Platform-specific scan code */
    VaxpKeyMod modifiers;
    VaxpBool is_repeat;
} VaxpKeyEvent;

typedef struct VaxpTextEvent {
    VaxpU32 window_id;
    char text[32];        /* UTF-8 encoded text */
} VaxpTextEvent;

typedef struct VaxpDndEvent {
    VaxpU32 window_id;       /* Target window ID */
    VaxpU32 source_window;   /* Source window ID (for ENTER) */
    VaxpI32 x;               /* X position relative to window */
    VaxpI32 y;               /* Y position relative to window */
    VaxpI32 root_x;          /* X position on screen */
    VaxpI32 root_y;          /* Y position on screen */
    VaxpU32 actions;         /* Supported actions bitmask */
    char** mime_types;        /* Available MIME types (for ENTER) */
    VaxpU32 mime_type_count; /* Number of MIME types */
} VaxpDndEvent;

/* ============================================================================
 * UNIFIED EVENT STRUCTURE
 * ============================================================================ */

typedef struct VaxpEvent {
    VaxpEventType type;
    VaxpU64 timestamp;   /* Event timestamp in milliseconds */
    
    union {
        VaxpWindowEvent window;
        VaxpMouseEvent mouse;
        VaxpScrollEvent scroll;
        VaxpKeyEvent key;
        VaxpTextEvent text;
        VaxpDndEvent dnd;    /* For DND events */
        void* user_data;      /* For VAXP_EVENT_USER */
    };
} VaxpEvent;

/* ============================================================================
 * EVENT HELPERS
 * ============================================================================ */

/**
 * @brief Get name of event type for debugging
 */
const char* vaxp_event_type_name(VaxpEventType type);

/**
 * @brief Check if event is a mouse event
 */
VAXP_INLINE VaxpBool vaxp_event_is_mouse(VaxpEventType type) {
    return type >= VAXP_EVENT_MOUSE_MOVE && type <= VAXP_EVENT_MOUSE_SCROLL;
}

/**
 * @brief Check if event is a keyboard event
 */
VAXP_INLINE VaxpBool vaxp_event_is_keyboard(VaxpEventType type) {
    return type >= VAXP_EVENT_KEY_DOWN && type <= VAXP_EVENT_TEXT_INPUT;
}

/**
 * @brief Check if event is a window event
 */
VAXP_INLINE VaxpBool vaxp_event_is_window(VaxpEventType type) {
    return type >= VAXP_EVENT_WINDOW_CLOSE && type <= VAXP_EVENT_WINDOW_HIDE;
}

/**
 * @brief Check if event is a drag and drop event
 */
VAXP_INLINE VaxpBool vaxp_event_is_dnd(VaxpEventType type) {
    return type >= VAXP_EVENT_DND_ENTER && type <= VAXP_EVENT_DND_DROP;
}

#ifdef __cplusplus
}
#endif

#endif /* VAXP_EVENT_H */
