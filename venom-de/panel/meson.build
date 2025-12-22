# Venom Panel
# Desktop panel for Venom Desktop Environment

# GIO for D-Bus communication with daemons
gio_dep = dependency('gio-2.0', required: true)

venom_panel = executable('venom-panel',
    'venom-panel.c',
    'audio_client.c',
    dependencies: [venomui_dep, gl_dep_for_demo, x11_dep_for_demo, m_dep_for_demo, gio_dep],
    install: true,
    install_dir: get_option('bindir'),
)
