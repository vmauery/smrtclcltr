
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
  dependencies: base_deps,
  include_directories: incdirs
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
  dependencies: base_deps,
  include_directories: incdirs
)

clcltr_src = [
  'calculator.cpp',
  'numeric.cpp',
  'stack_entry.cpp',
  'symbolic.cpp',
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
  include_directories: incdirs
  )

executable('smrtprsr',
  sources: 'parser_main.cpp',
  dependencies: base_deps,
  link_whole: common_lib,
  include_directories: incdirs
  )

