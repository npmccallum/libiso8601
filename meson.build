project('libiso8601', 'c',
    meson_version: '>0.36.0',
    license: 'ASL-2.0',
    version: '1'
)

# Global CFLAGS
size = meson.get_compiler('c').sizeof('time_t', prefix: '#include <time.h>')
add_project_arguments(
    '-DSIZEOF_TIME_T=@0@'.format(size),
    '-Wall',
    '-Wextra',
    '-Werror',
    '-Wstrict-aliasing',
    '-Wchar-subscripts',
    '-Wformat-security',
    '-Wmissing-declarations',
    '-Wmissing-prototypes',
    '-Wnested-externs',
    '-Wpointer-arith',
    '-Wshadow',
    '-Wsign-compare',
    '-Wstrict-prototypes',
    '-Wtype-limits',
    '-Wno-missing-field-initializers',
    '-Wno-unused-parameter',
    language: 'c'
)

# Libraries
install_headers('iso8601.h')
int = static_library('int', ['internal.c', 'internal.h'], pic: true)
map = 'libiso8601.map'
src = meson.current_source_dir()
scr = '-Wl,--version-script,@0@/@1@'.format(src, map)
iso = library('iso8601', ['parse.c', 'unparse.c', 'add.c', 'misc.c'],
    link_args: ['-Wl,--no-undefined', scr],
    link_depends: map,
    link_with: int,
    install: true
)

# Tests
test('internal', executable('t_internal', 't_internal.c', link_with: int))
test('unparse', executable('t_unparse', 't_unparse.c', link_with: iso))
test('parse', executable('t_parse', 't_parse.c', link_with: iso))
test('misc', executable('t_misc', 't_misc.c', link_with: iso))
test('add', executable('t_add', 't_add.c', link_with: iso))
