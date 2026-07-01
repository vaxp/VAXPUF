# Vaxp Panel
# Desktop panel for Vaxp Desktop Environment

# GIO for D-Bus communication with daemons
gio_dep = dependency('gio-2.0', required: true)

if enable_opengl
    vaxp_panel = executable('vaxp-panel',
        'vaxp-panel.c',
        'audio_client.c',
        dependencies: [vaxpui_dep, gl_dep, x11_dep, m_dep, gio_dep],
        install: true,
        install_dir: get_option('bindir'),
    )
endif
