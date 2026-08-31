project(
  'gitstat',
  'c',
  version: '1.0.0',
  default_options: [
    'c_std=c23',
    'warning_level=3',
  ]
)

add_project_arguments(
  '-D_DEFAULT_SOURCE',
  '-D_POSIX_C_SOURCE=200809L',
  language: 'c'
)

cc = meson.get_compiler('c')

c_args = []
if cc.has_argument('-std=c23')
  c_args += '-std=c23'
elif cc.has_argument('-std=c2x')
  c_args += '-std=c2x'
endif

curl_dep = dependency('libcurl', required: true)

inc = include_directories('include')

sources = files(
  'src/app/app.c',
  'src/app/context.c',
  'src/cli/args.c',
  'src/config/config.c',
  'src/config/ini.c',
  'src/http/client.c',
  'src/http/response.c',
  'src/json/json.c',
  'src/model/repository.c',
  'src/model/user_stats.c',
  'src/os/env.c',
  'src/os/paths.c',
  'src/os/platform.c',
  'src/os/process.c',
  'src/os/terminal.c',
  'src/provider/github.c',
  'src/provider/gitlab.c',
  'src/provider/provider.c',
  'src/render/formatter.c',
  'src/render/printer.c',
  'src/render/theme.c',
  'src/util/buffer.c',
  'src/util/memory.c',
  'src/util/string.c',
  'src/util/time.c',
  'src/error.c',
  'src/version.c',
  'src/main.c',
)

executable(
  'gitstat',
  sources,
  include_directories: inc,
  dependencies: [curl_dep],
  c_args: c_args,
  install: true,
)
