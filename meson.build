project('smrtclcltr', 'cpp',
  version : '1.0',
  meson_version: '>=1.3.0',
  default_options : [
    'werror=true',
    'warning_level=3',
    'buildtype=debug',
    'b_ndebug=if-release',
    'cpp_std=c++23',
    'b_lto_mode=default',
    'b_lto_threads=0',
    'b_lto=true',
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
  # boost has as return std::move(...) that causes an error with -Wall
  add_project_arguments(['-Wno-pessimizing-move'], language: 'cpp')
endif
build = get_option('buildtype')

# boost parser was introduced in boost-1.87
boost_dep = dependency('boost', version: '>=1.87.0', required: true)
#    boost_algorithm
#    boost_bimap
#    boost_integer
#    boost_math
#    boost_multiprecision
#    boost_random
#    boost_parser

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
    # boost has a deprecated thing
    '-Wno-deprecated-literal-operator',
    # '-Wunused', # boost has an unused variable... boo!
    '-Wunused-but-set-parameter',
    '-Wunused-but-set-variable',
    '-Wunused-const-variable ',
    '-Wunused-function',
    '-Wunused-label',
    '-Wunused-local-typedefs',
    '-Wunused-macros',
    '-Wno-unused-result',
    '-Wunused-value',
    '-Wno-unused-variable',
    '-Wno-unused-parameter',
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

incdirs = include_directories(['.', 'src'])

subdir('src/')
