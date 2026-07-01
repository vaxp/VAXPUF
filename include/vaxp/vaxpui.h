/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxpui.h - Main header that includes everything
 */

#ifndef VAXPUI_H
#define VAXPUI_H

/* Core */
#include "vaxp/core/vaxp_types.h"
#include "vaxp/core/vaxp_memory.h"
#include "vaxp/core/vaxp_result.h"
#include "vaxp/core/vaxp_ref.h"
#include "vaxp/core/vaxp_focus.h"
#include "vaxp/core/vaxp_theme.h"
#include "vaxp/core/vaxp_animation.h"
#include "vaxp/core/vaxp_animation_group.h"
#include "vaxp/core/vaxp_spring_animation.h"

/* Backend */
#include "vaxp/backend/vaxp_display.h"
#include "vaxp/backend/vaxp_event.h"
#include "vaxp/backend/vaxp_event_queue.h"
#include "vaxp/backend/vaxp_window.h"
#include "vaxp/backend/vaxp_surface.h"
#include "vaxp/backend/vaxp_dnd.h"
#include "vaxp/backend/vaxp_clipboard.h"
#include "vaxp/backend/vaxp_cursor.h"

/* Graphics */
#include "vaxp/graphics/vaxp_canvas.h"

/* Widgets */
#include "vaxp/widgets/vaxp_widget.h"
#include "vaxp/widgets/vaxp_button.h"
#include "vaxp/widgets/vaxp_label.h"
#include "vaxp/widgets/vaxp_container.h"
#include "vaxp/widgets/vaxp_text_input.h"
#include "vaxp/widgets/vaxp_scrollable.h"
#include "vaxp/widgets/vaxp_image.h"
#include "vaxp/widgets/vaxp_stack.h"
#include "vaxp/widgets/vaxp_spacer.h"
#include "vaxp/widgets/vaxp_divider.h"
#include "vaxp/widgets/vaxp_sized_box.h"
#include "vaxp/widgets/vaxp_padding.h"
#include "vaxp/widgets/vaxp_checkbox.h"
#include "vaxp/widgets/vaxp_switch.h"
#include "vaxp/widgets/vaxp_slider.h"
#include "vaxp/widgets/vaxp_progress.h"
#include "vaxp/widgets/vaxp_radio.h"
#include "vaxp/widgets/vaxp_tooltip.h"
#include "vaxp/widgets/vaxp_badge.h"
#include "vaxp/widgets/vaxp_list_view.h"
#include "vaxp/widgets/vaxp_dialog.h"
#include "vaxp/widgets/vaxp_context_menu.h"
#include "vaxp/widgets/vaxp_text_area.h"
#include "vaxp/widgets/vaxp_dropdown.h"
#include "vaxp/widgets/vaxp_tabs.h"
#include "vaxp/widgets/vaxp_card.h"
#include "vaxp/widgets/vaxp_notification.h"
#include "vaxp/widgets/vaxp_grid_view.h"
#include "vaxp/widgets/vaxp_avatar.h"
#include "vaxp/widgets/vaxp_icon.h"
#include "vaxp/widgets/vaxp_accordion.h"
#include "vaxp/widgets/vaxp_snackbar.h"
#include "vaxp/widgets/vaxp_chip.h"
#include "vaxp/widgets/vaxp_spinner.h"
#include "vaxp/widgets/vaxp_split_pane.h"
#include "vaxp/widgets/vaxp_tree_view.h"
#include "vaxp/widgets/vaxp_breadcrumb.h"
#include "vaxp/widgets/vaxp_search_bar.h"
#include "vaxp/widgets/vaxp_number_input.h"
#include "vaxp/widgets/vaxp_color_swatch.h"
#include "vaxp/widgets/vaxp_separator.h"
#include "vaxp/widgets/vaxp_toggle_button.h"
#include "vaxp/widgets/vaxp_skeleton.h"
#include "vaxp/widgets/vaxp_stepper.h"
#include "vaxp/widgets/vaxp_rating.h"
#include "vaxp/widgets/vaxp_link.h"
#include "vaxp/widgets/vaxp_popover.h"
#include "vaxp/widgets/vaxp_calendar.h"
#include "vaxp/widgets/vaxp_date_picker.h"
#include "vaxp/widgets/vaxp_time_picker.h"
#include "vaxp/widgets/vaxp_color_picker.h"
#include "vaxp/widgets/vaxp_file_chooser.h"
#include "vaxp/widgets/vaxp_appbar.h"
#include "vaxp/widgets/vaxp_bottom_nav.h"
#include "vaxp/widgets/vaxp_drawer.h"
#include "vaxp/widgets/vaxp_carousel.h"
#include "vaxp/widgets/vaxp_table.h"
#include "vaxp/widgets/vaxp_chart.h"
#include "vaxp/widgets/vaxp_menubar.h"
#include "vaxp/widgets/vaxp_toolbar.h"
#include "vaxp/widgets/vaxp_titlebar.h"

/* High-level API */
#include "vaxp/vaxp_app.h"

/* State Management (BLoC) */
#include "vaxp/state/vaxp_cubit.h"
#include "vaxp/state/vaxp_bloc_builder.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * LIBRARY INITIALIZATION
 * ============================================================================ */

/**
 * @brief Initialize VAXPUI library
 * 
 * Call before using any other VAXPUI functions.
 * Safe to call multiple times.
 */
VaxpResult vaxp_init(void);

/**
 * @brief Shutdown VAXPUI library
 * 
 * Releases all global resources. After this, vaxp_init() must be
 * called again before using the library.
 */
void vaxp_shutdown(void);

/**
 * @brief Get VAXPUI version string
 */
const char* vaxp_version_string(void);

#ifdef __cplusplus
}
#endif

#endif /* VAXPUI_H */
