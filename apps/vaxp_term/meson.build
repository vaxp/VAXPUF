vaxp_term_sources = files(
    'src/main.c',
    'src/pty_handler.c',
    'src/vaxp_terminal.c',
    'src/config_parser.c'
)

# find libutil for forkpty
util_dep = meson.get_compiler('c').find_library('util', required: true)

vaxp_term_exe = executable('vaxp_term',
    vaxp_term_sources,
    dependencies: [vaxpui_dep, util_dep],
    install: true,
)
