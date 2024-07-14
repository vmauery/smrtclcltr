project('smrtclcltr', 'cpp',
  version : '1.0',
  meson_version: '>=1.3.0',
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
add_project_arguments(['-Wno-unused-parameter'], language: 'cpp')

if (cxx.get_id() == 'gcc')
  add_project_arguments(['-fpermissive'], language: 'cpp')
endif
build = get_option('buildtype')

# boost (1.77-1.84) has complex_adaptor issues and will not build
# new clang and boost-1.76 works, but has lots of warnings. 1.85 is better
boost_dep = dependency('boost', version: '>=1.74.0', required: true)
#    boost_algorithm
#    boost_bimap
#    boost_integer
#    boost_math
#    boost_multiprecision
#    boost_random
#    boost_spirit
boost_ver = boost_dep.version()
if boost_ver.version_compare('>=1.77') and boost_ver.version_compare('<=1.84')
  error('boost 1.77 through 1.84 do not build correctly')
endif

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

base_deps = [ numeric, boost_dep ]

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
    # needed for new clang for boost warnings
    '-Wno-double-promotion',
    # end boost list
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

smrty_functions = [
  'function_library.cpp',
  'constants.cpp',
  'functions/arithmetic_funcs.cpp',
  'functions/bitwise_funcs.cpp',
  'functions/compound.cpp',
  'functions/factorial.cpp',
  'functions/float.cpp',
  'functions/hyp_trig_funcs.cpp',
  'functions/integer.cpp',
  'functions/log.cpp',
  'functions/matrix.cpp',
  'functions/mode_funcs.cpp',
  'functions/modular_funcs.cpp',
  'functions/probability.cpp',
  'functions/product.cpp',
  'functions/program.cpp',
  'functions/range.cpp',
  'functions/sign.cpp',
  'functions/sqr.cpp',
  'functions/sqrt.cpp',
  'functions/stack_funcs.cpp',
  'functions/sum.cpp',
  'functions/time_funcs.cpp',
  'functions/trig_funcs.cpp',
  'functions/unit_funcs.cpp',
  ]

smrty_lib = static_library(
  'smtrty',
  smrty_functions,
  dependencies: base_deps
)

common_src = [
  'debug.cpp',
  'input.cpp',
  'parser.cpp',
  'program.cpp',
  ]

common_lib = static_library(
  'common',
  common_src,
  dependencies: base_deps
)

clcltr_src = [
  'calculator.cpp',
  'numeric.cpp',
  'stack_entry.cpp',
  'units.cpp',
  version_src,
  ]

cli_deps = base_deps
cli_src = [
  'main_cli.cpp',
  ]
cli_src += clcltr_src

executable('smrtclcltr',
  sources: cli_src,
  dependencies: cli_deps,
  link_whole: [smrty_lib, common_lib],
  )

executable('smrtprsr',
  sources: 'parser_main.cpp',
  dependencies: base_deps,
  link_whole: common_lib,
  )
