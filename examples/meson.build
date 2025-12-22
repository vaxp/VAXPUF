# Examples

hello_world_exe = executable('hello_world',
    'hello_world.c',
    dependencies: venomui_dep,
)

simple_app_exe = executable('simple_app',
    'simple_app.c',
    dependencies: venomui_dep,
)

counter_app_exe = executable('counter_app',
    'counter_app.c',
    dependencies: venomui_dep,
)

multi_window_exe = executable('multi_window',
    'multi_window.c',
    dependencies: venomui_dep,
)

text_input_demo_exe = executable('text_input_demo',
    'text_input_demo.c',
    dependencies: venomui_dep,
)

widget_test_exe = executable('widget_test',
    'widget_test.c',
    dependencies: venomui_dep,
)

animation_demo_exe = executable('animation_demo',
    'animation_demo.c',
    dependencies: venomui_dep,
)

const_demo_exe = executable('const_demo',
    'const_demo.c',
    dependencies: venomui_dep,
)

new_widgets_demo_exe = executable('new_widgets_demo',
    'new_widgets_demo.c',
    dependencies: venomui_dep,
)

widget_panel_demo_exe = executable('widget_panel_demo',
    'widget_panel_demo.c',
    dependencies: venomui_dep,
)

# OpenGL demo - only build when OpenGL backend is enabled
if enable_opengl
    gl_dep_for_demo = dependency('gl', required: true)
    x11_dep_for_demo = dependency('x11', required: true)
    m_dep_for_demo = meson.get_compiler('c').find_library('m', required: true)
    
    opengl_demo_exe = executable('opengl_demo',
        'opengl_demo.c',
        dependencies: [venomui_dep, gl_dep_for_demo, x11_dep_for_demo, m_dep_for_demo],
    )
    
    panel_demo_exe = executable('panel_demo',
        'panel_demo.c',
        dependencies: [venomui_dep, gl_dep_for_demo, x11_dep_for_demo, m_dep_for_demo],
    )
endif
