/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venomui.h - Main header that includes everything
 */

#ifndef VENOMUI_H
#define VENOMUI_H

/* Core */
#include "venom/core/venom_types.h"
#include "venom/core/venom_memory.h"
#include "venom/core/venom_result.h"
#include "venom/core/venom_ref.h"
#include "venom/core/venom_focus.h"
#include "venom/core/venom_theme.h"

/* Backend */
#include "venom/backend/venom_display.h"
#include "venom/backend/venom_event.h"
#include "venom/backend/venom_window.h"
#include "venom/backend/venom_surface.h"

/* Graphics */
#include "venom/graphics/venom_canvas.h"

/* Widgets */
#include "venom/widgets/venom_widget.h"
#include "venom/widgets/venom_button.h"
#include "venom/widgets/venom_label.h"
#include "venom/widgets/venom_container.h"
#include "venom/widgets/venom_text_input.h"
#include "venom/widgets/venom_scrollable.h"
#include "venom/widgets/venom_image.h"
#include "venom/widgets/venom_stack.h"
#include "venom/widgets/venom_spacer.h"
#include "venom/widgets/venom_divider.h"
#include "venom/widgets/venom_sized_box.h"
#include "venom/widgets/venom_padding.h"
#include "venom/widgets/venom_checkbox.h"
#include "venom/widgets/venom_switch.h"
#include "venom/widgets/venom_slider.h"
#include "venom/widgets/venom_progress.h"
#include "venom/widgets/venom_radio.h"
#include "venom/widgets/venom_tooltip.h"
#include "venom/widgets/venom_badge.h"
#include "venom/widgets/venom_list_view.h"
#include "venom/widgets/venom_dialog.h"
#include "venom/widgets/venom_context_menu.h"

/* High-level API */
#include "venom/venom_app.h"

/* State Management (BLoC) */
#include "venom/state/venom_cubit.h"
#include "venom/state/venom_bloc_builder.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * LIBRARY INITIALIZATION
 * ============================================================================ */

/**
 * @brief Initialize VENOMUI library
 * 
 * Call before using any other VENOMUI functions.
 * Safe to call multiple times.
 */
VenomResult venom_init(void);

/**
 * @brief Shutdown VENOMUI library
 * 
 * Releases all global resources. After this, venom_init() must be
 * called again before using the library.
 */
void venom_shutdown(void);

/**
 * @brief Get VENOMUI version string
 */
const char* venom_version_string(void);

#ifdef __cplusplus
}
#endif

#endif /* VENOMUI_H */
