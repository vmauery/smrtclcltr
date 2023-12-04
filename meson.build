project('smrtclcltr', 'cpp',
  version : '1.0',
  meson_version: '>=1.1.0',
  default_options : [
    'werror=true',
    'warning_level=3',
    'buildtype=debugoptimized',
    'b_ndebug=if-release',
    'cpp_std=c++23',
    ]
  )

version_src = vcs_tag(
  command: 'git-show-version.sh',
  input: 'version.cpp.in',
  output: 'version.cpp',
  replace_string: 'GIT_VERSION_STRING',
  )

ts_cmd = run_command('date', '-Iseconds', check: true)
timestamp_iso8601 = ts_cmd.stdout().strip()
add_project_arguments(['-D__TIMESTAMP_ISO8601__="'+timestamp_iso8601+'"'], language: 'cpp')

cxx = meson.get_compiler('cpp')
if (cxx.get_id() == 'gcc')
  add_project_arguments(['-fpermissive'], language: 'cpp')
endif
build = get_option('buildtype')

numeric = dependency('mpfr')
if (get_option('numeric') == 'mpfr')
  numeric = dependency('mpfr')
  add_project_arguments(['-DUSE_MPFR_BACKEND'], language: 'cpp')
elif (get_option('numeric') == 'gmp')
  numeric = dependency('gmp')
  add_project_arguments(['-DUSE_GMP_BACKEND'], language: 'cpp')
elif (get_option('numeric') == 'boost')
  add_project_arguments(['-DUSE_BOOST_CPP_BACKEND'], language: 'cpp')
elif (get_option('numeric') == 'native')
  add_project_arguments(['-DUSE_BASIC_TYPES'], language: 'cpp')
endif

base_deps = [ numeric ]

add_project_arguments(
  cxx.get_supported_arguments([
    '-Wcast-align',
    '-Wdouble-promotion',
    '-Wformat=2',
    '-Wmisleading-indentation',
    '-Wno-reorder',
    '-Wnon-virtual-dtor',
    # '-Wnull-dereference', # gcc gives false positives
    '-Woverloaded-virtual',
    '-Wunused',
    ]),
  language: 'cpp'
  )
# Set Compiler Security flags

security_flags = [
  '-fstack-protector-strong',
  '-fPIE',
  '-fPIC',
  '-D_FORTIFY_SOURCE=2',
  '-Wformat',
  '-Wformat-security'
  ]

# Use readline by default if found, can be disabled with option
rldep = dependency('readline', required: false)
if rldep.found() and get_option('readline') == true
  add_project_arguments(['-DHAVE_READLINE'], language: 'cpp')
  base_deps += [ rldep ]
endif

## Add security flags for builds of type 'release','debugoptimized' and 'minsize'

if not (get_option('buildtype') == 'plain' or get_option('buildtype').startswith('debug'))
  add_project_arguments(
    cxx.get_supported_arguments([
      security_flags
      ]),
    language: 'cpp')
endif

# Boost dependency configuration

add_project_arguments(
  [
    '-DBOOST_ALL_NO_LIB',
    '-DBOOST_ASIO_DISABLE_THREADS',
    '-DBOOST_ERROR_CODE_HEADER_ONLY',
    '-DBOOST_NO_RTTI',
    '-DBOOST_NO_TYPEID',
    '-DBOOST_SYSTEM_NO_DEPRECATED'
    ],
  language : 'cpp')

smrty_src = [
  'calculator.cpp',
  'constants.cpp',
  'debug.cpp',
  'input.cpp',
  'numeric.cpp',
  'parser.cpp',
  'units.cpp',
  version_src,
  'functions/arithmetic_funcs.cpp',
  'functions/bitwise_funcs.cpp',
  'functions/factorial.cpp',
  'functions/integer.cpp',
  'functions/log.cpp',
  'functions/matrix.cpp',
  'functions/mode_funcs.cpp',
  'functions/modular_funcs.cpp',
  'functions/probability.cpp',
  'functions/range.cpp',
  'functions/sqr.cpp',
  'functions/sqrt.cpp',
  'functions/stack_funcs.cpp',
  'functions/time_funcs.cpp',
  'functions/trig_funcs.cpp',
  'functions/unit_funcs.cpp',
  'functions/util_funcs.cpp',
  ]

smrty_lib = static_library('smtrty', smrty_src)

cli_deps = base_deps
cli_src = [
  'main_cli.cpp',
  ]

executable('smrtclcltr',
  sources: cli_src,
  dependencies: cli_deps,
  link_whole: smrty_lib,
  )
