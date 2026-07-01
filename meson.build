project('vaxpui', ['c', 'cpp'],
    version: '0.1.0',
    license: 'MIT',
    meson_version: '>=0.56.0',
    default_options: [
        'c_std=c11',
        'cpp_std=c++17',
        'warning_level=3',
        'buildtype=debugoptimized',
    ]
)

# Options
build_examples = get_option('examples')
build_tests = get_option('tests')
enable_debug = get_option('debug_memory')
enable_opengl = get_option('opengl')

# Dependencies
x11_dep = dependency('x11', required: true)
xcb_dep = dependency('xcb', required: false)

# OpenGL dependencies (optional)
gl_dep = dependency('gl', required: false)
glx_dep = dependency('glx', required: false)  # Part of X11/OpenGL

# Skia dependency (may need custom handling)
skia_dep = dependency('skia', required: false)

# Fallback to Cairo if Skia not available
cairo_dep = dependency('cairo', required: false)

# PangoCairo (still needed for Cairo backend)
pangocairo_dep = dependency('pangocairo', required: false)

# Freetype, Harfbuzz, and Fontconfig for native OpenGL text engine
freetype_dep = dependency('freetype2', required: false)
harfbuzz_dep = dependency('harfbuzz', required: false)
fontconfig_dep = dependency('fontconfig', required: false)

# Check for valid backend
if enable_opengl
    if not gl_dep.found()
        error('OpenGL backend requested but OpenGL not found')
    endif
elif not skia_dep.found() and not cairo_dep.found()
    error('Either Skia, Cairo, or OpenGL is required for rendering')
endif

# Thread support for atomic operations
thread_dep = dependency('threads')

# libpng for image loading
png_dep = dependency('libpng', required: true)

# Math library (for sinf, cosf, etc.)
m_dep = meson.get_compiler('c').find_library('m', required: false)

# Compiler flags
c_args = []
cpp_args = []

if enable_debug
    c_args += ['-DVAXP_DEBUG=1']
    cpp_args += ['-DVAXP_DEBUG=1']
endif

if enable_opengl
    c_args += ['-DVAXP_USE_OPENGL=1']
endif

# Include directories
inc_dirs = include_directories('include')

# Source files
core_sources = files(
    'src/core/vaxp_memory.c',
    'src/core/vaxp_ref.c',
    'src/core/vaxp_result.c',
    'src/core/vaxp_init.c',
    'src/core/vaxp_app.c',
    'src/core/vaxp_focus.c',
    'src/core/vaxp_theme.c',
    'src/core/vaxp_animation.c',
    'src/core/vaxp_animation_group.c',
    'src/core/vaxp_spring_animation.c',
)

backend_sources = files(
    'src/backend/x11/vaxp_x11_display.c',
    'src/backend/x11/vaxp_event.c',
    'src/backend/x11/vaxp_x11_dnd.c',
    'src/backend/x11/vaxp_x11_clipboard.c',
    'src/backend/x11/vaxp_x11_cursor.c',
    'src/backend/vaxp_window.c',
    'src/backend/vaxp_event_queue.c',
)

widget_sources = files(
    'src/widgets/vaxp_widget.c',
    'src/widgets/vaxp_button.c',
    'src/widgets/vaxp_label.c',
    'src/widgets/vaxp_container.c',
    'src/widgets/vaxp_text_input.c',
    'src/widgets/vaxp_scrollable.c',
    'src/widgets/vaxp_image.c',
    'src/widgets/vaxp_stack.c',
    'src/widgets/vaxp_spacer.c',
    'src/widgets/vaxp_divider.c',
    'src/widgets/vaxp_sized_box.c',
    'src/widgets/vaxp_padding.c',
    'src/widgets/vaxp_checkbox.c',
    'src/widgets/vaxp_switch.c',
    'src/widgets/vaxp_slider.c',
    'src/widgets/vaxp_progress.c',
    'src/widgets/vaxp_radio.c',
    'src/widgets/vaxp_tooltip.c',
    'src/widgets/vaxp_badge.c',
    'src/widgets/vaxp_list_view.c',
    'src/widgets/vaxp_dialog.c',
    'src/widgets/vaxp_context_menu.c',
    'src/widgets/vaxp_text_area.c',
    'src/widgets/vaxp_dropdown.c',
    'src/widgets/vaxp_tabs.c',
    'src/widgets/vaxp_card.c',
    'src/widgets/vaxp_notification.c',
    'src/widgets/vaxp_grid_view.c',
    'src/widgets/vaxp_avatar.c',
    'src/widgets/vaxp_icon.c',
    'src/widgets/vaxp_accordion.c',
    'src/widgets/vaxp_snackbar.c',
    'src/widgets/vaxp_chip.c',
    'src/widgets/vaxp_spinner.c',
    'src/widgets/vaxp_split_pane.c',
    'src/widgets/vaxp_tree_view.c',
    'src/widgets/vaxp_breadcrumb.c',
    'src/widgets/vaxp_search_bar.c',
    'src/widgets/vaxp_number_input.c',
    'src/widgets/vaxp_color_swatch.c',
    'src/widgets/vaxp_toggle_button.c',
    'src/widgets/vaxp_skeleton.c',
    'src/widgets/vaxp_stepper.c',
    'src/widgets/vaxp_rating.c',
    'src/widgets/vaxp_link.c',
    'src/widgets/vaxp_popover.c',
    'src/widgets/vaxp_calendar.c',
    'src/widgets/vaxp_date_picker.c',
    'src/widgets/vaxp_time_picker.c',
    'src/widgets/vaxp_color_picker.c',
    'src/widgets/vaxp_file_chooser.c',
    'src/widgets/vaxp_appbar.c',
    'src/widgets/vaxp_bottom_nav.c',
    'src/widgets/vaxp_drawer.c',
    'src/widgets/vaxp_carousel.c',
    'src/widgets/vaxp_table.c',
    'src/widgets/vaxp_chart.c',
    'src/widgets/vaxp_menubar.c',
    'src/widgets/vaxp_toolbar.c',
    'src/widgets/vaxp_titlebar.c',
)

state_sources = files(
    'src/state/vaxp_cubit.c',
    'src/state/vaxp_bloc_builder.c',
)

# Graphics sources depend on available backend
if enable_opengl
    # OpenGL mode: use OpenGL canvas but also keep Cairo for image widgets
    graphics_sources = files(
        'src/graphics/vaxp_canvas_opengl.c',
        'src/graphics/vaxp_canvas_cairo.c'  # Still needed for app integration
    )
    graphics_dep = gl_dep
    graphics_extra_deps = [cairo_dep, pangocairo_dep, freetype_dep, harfbuzz_dep, fontconfig_dep]
elif skia_dep.found()
    graphics_sources = files('src/graphics/vaxp_canvas_skia.cpp')
    graphics_dep = skia_dep
    graphics_extra_deps = [pangocairo_dep]
else
    # Cairo fallback implementation
    graphics_sources = files('src/graphics/vaxp_canvas_cairo.c')
    graphics_dep = cairo_dep
    graphics_extra_deps = [pangocairo_dep]
endif

all_sources = [
    core_sources,
    backend_sources,
    widget_sources,
    state_sources,
    graphics_sources,
]

# Build library
vaxpui_lib = library('vaxpui',
    all_sources,
    include_directories: inc_dirs,
    dependencies: [x11_dep, graphics_dep, png_dep, thread_dep, m_dep] + graphics_extra_deps,
    c_args: c_args,
    cpp_args: cpp_args,
    install: true,
)

# Declare dependency for use by examples
vaxpui_dep = declare_dependency(
    include_directories: inc_dirs,
    link_with: vaxpui_lib,
)

# Install headers
install_subdir('include/vaxp', install_dir: get_option('includedir'))

# Generate pkg-config file
pkg = import('pkgconfig')
pkg.generate(vaxpui_lib,
    name: 'vaxpui',
    description: 'High Performance GUI Framework',
    version: meson.project_version(),
)

# Examples
if build_examples
    subdir('examples')
endif

# Tests
if build_tests
    subdir('tests')
endif

# Vaxp Desktop Environment
subdir('vaxp-de')

# Summary
backend_name = 'Cairo'
if enable_opengl
    backend_name = 'OpenGL'
elif skia_dep.found()
    backend_name = 'Skia'
endif

summary({
    'Backend': backend_name,
    'Debug Memory': enable_debug,
    'Build Examples': build_examples,
    'Build Tests': build_tests,
}, section: 'Configuration')
