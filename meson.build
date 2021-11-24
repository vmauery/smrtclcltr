project('smrtclcltr', 'cpp',
  version : '1.0',
  meson_version: '>=0.53.2',
  default_options : [
    'werror=true',
    'warning_level=3',
    'buildtype=debugoptimized',
    'b_ndebug=if-release',
    'cpp_std=c++2a',
    ]
  )

src = [
  'main.cpp',
  'numeric.cpp',
  'constants.cpp',
  'calculator.cpp',
  'functions/arithmetic_funcs.cpp',
  'functions/fn_template.cpp',
  'functions/probability.cpp',
  'functions/sqrt.cpp',
  'functions/trig_funcs.cpp',
  'functions/bitwise_funcs.cpp',
  'functions/integer.cpp',
  'functions/product.cpp',
  'functions/stack_funcs.cpp',
  'functions/factorial.cpp',
  'functions/log.cpp',
  'functions/range.cpp',
  'functions/sum.cpp',
  ]

cxx = meson.get_compiler('cpp')
build = get_option('buildtype')

numeric = dependency('boost')
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

add_project_arguments(
  cxx.get_supported_arguments([
    '-Wcast-align',
    '-Wdouble-promotion',
    '-Wformat=2',
    '-Wmisleading-indentation',
    '-Wno-reorder',
    '-Wnon-virtual-dtor',
    '-Wnull-dereference',
    '-Wold-style-cast',
    '-Woverloaded-virtual',
    '-Wunused',
    '-std=c++20',
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


executable('smrtclcltr',
  sources: src,
  dependencies: numeric
  )
