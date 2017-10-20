project('libiso8601', 'c',
    meson_version: '>0.36.0',
    license: 'ASL-2.0',
    version: '1'
)

cc = meson.get_compiler('c')
lnk = []

# Test for sizeof(time_t)
size = cc.sizeof('time_t', prefix: '#include <time.h>')

# Test for '-Wl,--no-undefined'
arg = '-Wl,--no-undefined'
if cc.links('int main (void) { return 0; }', args: arg)
lnk += arg
endif

# Test for '-Wl,--version-script,...'
map = 'libiso8601.map'
src = meson.current_source_dir()
arg = '-Wl,--version-script,' + join_paths(src, map)
if cc.links('int main (void) { return 0; }', args: arg)
lnk += arg
endif

# Global CFLAGS
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
    '-Wno-sign-compare',
    '-Wstrict-prototypes',
    '-Wtype-limits',
    '-Wno-missing-field-initializers',
    '-Wno-unused-parameter',
    language: 'c'
)

# Libraries
install_headers('iso8601.h')
int = static_library('int', ['internal.c', 'internal.h'], pic: true)
iso = library('iso8601', ['parse.c', 'unparse.c', 'add.c', 'misc.c'],
    link_depends: map,
    link_args: lnk,
    link_with: int,
    install: true
)

# PkgConfig
pkg = import('pkgconfig')
pkg.generate(
    libraries: iso,
    name: meson.project_name(),
    version: meson.project_version(),
    description: 'A library for parsing ISO 8601 dates.'
)

# Tests
test('internal', executable('t_internal', 't_internal.c', link_with: int))
test('unparse', executable('t_unparse', 't_unparse.c', link_with: iso))
test('parse', executable('t_parse', 't_parse.c', link_with: iso))
test('misc', executable('t_misc', 't_misc.c', link_with: iso))
test('add', executable('t_add', 't_add.c', link_with: iso))
