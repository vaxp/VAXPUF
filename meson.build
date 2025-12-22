project('venomui', ['c', 'cpp'],
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

# PangoCairo for proper international text rendering (Arabic, CJK, etc.)
pangocairo_dep = dependency('pangocairo', required: false)

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
    c_args += ['-DVENOM_DEBUG=1']
    cpp_args += ['-DVENOM_DEBUG=1']
endif

if enable_opengl
    c_args += ['-DVENOM_USE_OPENGL=1']
endif

# Include directories
inc_dirs = include_directories('include')

# Source files
core_sources = files(
    'src/core/venom_memory.c',
    'src/core/venom_ref.c',
    'src/core/venom_result.c',
    'src/core/venom_init.c',
    'src/core/venom_app.c',
    'src/core/venom_focus.c',
    'src/core/venom_theme.c',
    'src/core/venom_animation.c',
    'src/core/venom_animation_group.c',
    'src/core/venom_spring_animation.c',
)

backend_sources = files(
    'src/backend/x11/venom_x11_display.c',
    'src/backend/x11/venom_event.c',
    'src/backend/venom_window.c',
    'src/backend/venom_event_queue.c',
)

widget_sources = files(
    'src/widgets/venom_widget.c',
    'src/widgets/venom_button.c',
    'src/widgets/venom_label.c',
    'src/widgets/venom_container.c',
    'src/widgets/venom_text_input.c',
    'src/widgets/venom_scrollable.c',
    'src/widgets/venom_image.c',
    'src/widgets/venom_stack.c',
    'src/widgets/venom_spacer.c',
    'src/widgets/venom_divider.c',
    'src/widgets/venom_sized_box.c',
    'src/widgets/venom_padding.c',
    'src/widgets/venom_checkbox.c',
    'src/widgets/venom_switch.c',
    'src/widgets/venom_slider.c',
    'src/widgets/venom_progress.c',
    'src/widgets/venom_radio.c',
    'src/widgets/venom_tooltip.c',
    'src/widgets/venom_badge.c',
    'src/widgets/venom_list_view.c',
    'src/widgets/venom_dialog.c',
    'src/widgets/venom_context_menu.c',
    'src/widgets/venom_text_area.c',
    'src/widgets/venom_dropdown.c',
    'src/widgets/venom_tabs.c',
    'src/widgets/venom_card.c',
    'src/widgets/venom_notification.c',
    'src/widgets/venom_grid_view.c',
    'src/widgets/venom_avatar.c',
    'src/widgets/venom_icon.c',
    'src/widgets/venom_accordion.c',
    'src/widgets/venom_snackbar.c',
    'src/widgets/venom_chip.c',
    'src/widgets/venom_spinner.c',
    'src/widgets/venom_split_pane.c',
    'src/widgets/venom_tree_view.c',
    'src/widgets/venom_breadcrumb.c',
    'src/widgets/venom_search_bar.c',
    'src/widgets/venom_number_input.c',
    'src/widgets/venom_color_swatch.c',
    'src/widgets/venom_toggle_button.c',
    'src/widgets/venom_skeleton.c',
    'src/widgets/venom_stepper.c',
    'src/widgets/venom_rating.c',
    'src/widgets/venom_link.c',
    'src/widgets/venom_popover.c',
    'src/widgets/venom_calendar.c',
    'src/widgets/venom_date_picker.c',
    'src/widgets/venom_time_picker.c',
    'src/widgets/venom_color_picker.c',
    'src/widgets/venom_file_chooser.c',
    'src/widgets/venom_appbar.c',
    'src/widgets/venom_bottom_nav.c',
    'src/widgets/venom_drawer.c',
    'src/widgets/venom_carousel.c',
    'src/widgets/venom_table.c',
    'src/widgets/venom_chart.c',
)

state_sources = files(
    'src/state/venom_cubit.c',
    'src/state/venom_bloc_builder.c',
)

# Graphics sources depend on available backend
if enable_opengl
    # OpenGL mode: use OpenGL canvas but also keep Cairo for image widgets
    graphics_sources = files(
        'src/graphics/venom_canvas_opengl.c',
        'src/graphics/venom_canvas_cairo.c'  # Still needed for app integration
    )
    graphics_dep = gl_dep
    graphics_extra_deps = [cairo_dep, pangocairo_dep]
elif skia_dep.found()
    graphics_sources = files('src/graphics/venom_canvas_skia.cpp')
    graphics_dep = skia_dep
    graphics_extra_deps = [pangocairo_dep]
else
    # Cairo fallback implementation
    graphics_sources = files('src/graphics/venom_canvas_cairo.c')
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
venomui_lib = library('venomui',
    all_sources,
    include_directories: inc_dirs,
    dependencies: [x11_dep, graphics_dep, png_dep, thread_dep, m_dep] + graphics_extra_deps,
    c_args: c_args,
    cpp_args: cpp_args,
    install: true,
)

# Declare dependency for use by examples
venomui_dep = declare_dependency(
    include_directories: inc_dirs,
    link_with: venomui_lib,
)

# Install headers
install_subdir('include/venom', install_dir: get_option('includedir'))

# Generate pkg-config file
pkg = import('pkgconfig')
pkg.generate(venomui_lib,
    name: 'venomui',
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
