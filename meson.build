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

# Dependencies
x11_dep = dependency('x11', required: true)
xcb_dep = dependency('xcb', required: false)

# Skia dependency (may need custom handling)
skia_dep = dependency('skia', required: false)

# Fallback to Cairo if Skia not available
cairo_dep = dependency('cairo', required: false)

# PangoCairo for proper international text rendering (Arabic, CJK, etc.)
pangocairo_dep = dependency('pangocairo', required: true)

if not skia_dep.found() and not cairo_dep.found()
    error('Either Skia or Cairo is required for rendering')
endif

# Thread support for atomic operations
thread_dep = dependency('threads')

# libpng for image loading
png_dep = dependency('libpng', required: true)

# Compiler flags
c_args = []
cpp_args = []

if enable_debug
    c_args += ['-DVENOM_DEBUG=1']
    cpp_args += ['-DVENOM_DEBUG=1']
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
)

backend_sources = files(
    'src/backend/x11/venom_x11_display.c',
    'src/backend/x11/venom_event.c',
    'src/backend/venom_window.c',
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
)

state_sources = files(
    'src/state/venom_cubit.c',
    'src/state/venom_bloc_builder.c',
)

# Graphics sources depend on available backend
if skia_dep.found()
    graphics_sources = files('src/graphics/venom_canvas_skia.cpp')
    graphics_dep = skia_dep
else
    # Cairo fallback implementation
    graphics_sources = files('src/graphics/venom_canvas_cairo.c')
    graphics_dep = cairo_dep
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
    dependencies: [x11_dep, graphics_dep, pangocairo_dep, png_dep, thread_dep],
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
summary({
    'Backend': skia_dep.found() ? 'Skia' : 'Cairo',
    'Debug Memory': enable_debug,
    'Build Examples': build_examples,
    'Build Tests': build_tests,
}, section: 'Configuration')
