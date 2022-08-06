project('tsp', 'cpp',
    version : '0.1',
    default_options : ['warning_level=3',
                     'cpp_std=c++20'])

build_arguments = []
link_arguments = []

ccpp = meson.get_compiler('cpp')
ccppid = ccpp.get_id()

if get_option('native')
    if ccppid == 'clang' or ccppid == 'gcc'
        build_arguments += ['-march=native']
    endif
endif

if get_option('libcpp') and ccppid == 'clang'
    if ccpp.find_library('libc++', required: true).found()
        build_arguments += ['-stdlib=libc++']
        link_arguments += ['-stdlib=libc++']
    endif
endif               

subdir('src')
subdir('tests')
