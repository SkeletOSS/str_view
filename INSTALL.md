# Building and Installation

## Fetch Content

This approach will allow CMake to build `str_view` from source as part of your project. It does not have external dependencies, besides the standard library, so this may be viable for you. This is helpful if you want the ability to build the library in release or debug mode along with your project and possibly step through it with a debugger during a debug build. If you would rather link to the release build library file see the next section for the manual install.

To avoid including tests, samples, and other extraneous files when fetching content download a release.

```cmake
include(FetchContent)
FetchContent_Declare(
  str_view 
  URL https://github.com/skeletoss/str_view/releases/download/v[MAJOR.MINOR.PATCH]/str_view-v[MAJOR.MINOR.PATCH].zip
  #DOWNLOAD_EXTRACT_TIMESTAMP FALSE # CMake may raise a warning to set this. If so, uncomment and set.
  URL_HASH SHA256=[HASH]
  SYSTEM # Optional flag to mark ccc as a system library and silence compiler and tooling warnings.
)
FetchContent_MakeAvailable(str_view)
# Optionally ignore compiler warnings from the str_view library.
target_compile_options(str_view PRIVATE "-w")
```

Link against the library with its namespace.

```cmake
add_executable(main main.c)
target_link_libraries(main str_view::strrview)
```

Here is a concrete example with an arbitrary release that is likely out of date. Replace this version with the newest version on the releases page.

```cmake
include(FetchContent)
FetchContent_Declare(
  str_view 
  URL https://github.com/skeletoss/str_view/releases/download/v0.7.2/str_view-v0.7.2.zip
  #DOWNLOAD_EXTRACT_TIMESTAMP FALSE # CMake may raise a warning to set this. If so, uncomment and set.
  URL_HASH SHA256=cbe9aa4f416ea13502cd48d089e5cb67d7f3552772b0a88773f62a9fe4cf1a5a
  SYSTEM # Optional flag to mark ccc as a system library and silence compiler and tooling warnings.
)
FetchContent_MakeAvailable(str_view)
# Optionally ignore compiler warnings from the str_view library.
target_compile_options(str_view PRIVATE "-w")
get_target_property(str_view_SOURCE_DIR str_view SOURCE_DIR)
add_executable(main main.c)
# Optionally include str_view source directory as system so that your tooling like clang-tidy ignores it.
target_include_directories(main SYSTEM PRIVATE ${str_view_SOURCE_DIR})
target_link_libraries(main str_view::str_view)
```

Now, `str_view` is part of your project build, allowing you to configure as you see fit. For a more traditional approach read the manual install section below.

## Freestanding Environments

The string view library uses the following functions or macros that must be supported by the user on freestanding targets.

Traditionally included via `<string.h>`:

- `memmove()`
- `memcmp()`
- `strlen()`
- `strnlen()`

To provide these functions, the user may create a header. For example, `my_sv_configuration.h`.

```txt
my_project/
    my_sv_configuration/
        my_sv_configuration.h
```

In this header the user has two options: provide the listed functions directly or include their versions of the headers that provide the needed functionality. It is common for freestanding environments to provide their own `<string.h>` that implement these functions.

```c
#ifndef MY_SV_CONFIGURATION_H
#define MY_SV_CONFIGURATION_H
#include "lib/string.h" /* IWYU pragma: export */
#endif /* MY_SV_CONFIGURATION_H */
```

The Include What You Use (IWYU) comment is helpful if you want to avoid tooling warnings in SV code and have not marked SV code as a system library to silence such warnings. If you are providing functions directly, this is not applicable. Then, ensure the string view library can find that header.

```cmake
include(FetchContent)
FetchContent_Declare(
  str_view 
  URL https://github.com/skeletoss/str_view/releases/download/v0.7.2/str_view-v0.7.2.zip
  #DOWNLOAD_EXTRACT_TIMESTAMP FALSE # CMake may raise a warning to set this. If so, uncomment and set.
  URL_HASH SHA256=cbe9aa4f416ea13502cd48d089e5cb67d7f3552772b0a88773f62a9fe4cf1a5a
  SYSTEM # Optional flag to mark ccc as a system library and silence compiler and tooling warnings.
)
FetchContent_MakeAvailable(str_view)
# Include this line if you want to ignore compiler warnings from the ccc library when compiling your project.
target_compile_options(ccc PRIVATE "-w")

# New step allowing CCC to find the configuration header.
target_include_directories(str_view PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/my_sv_configuration>
)

add_executable(freestanding freestanding.c)
target_link_libraries(freestanding str_view::str_view)
```

Now pass the flag to CMake at configure time via `CMakePresets.json`, `CMakeUserPresets.json`, or the command line.

```json
"cacheVariables": {
    "SV_USER_CONFIGURATION": "my_sv_configuration.h",
}
```

Or they can be passed on the command line.

```zsh
cmake --preset=my-preset\
 -DSV_USER_CONFIGURATION="my_sv_configuration.h"\
```

Now the library is fully configured to be built as part of the user project in a freestanding environment.

Any other C headers that the collection uses internally, such as `<stdint.h>` and `<stddef.h>`, are those provided by the C standard on freestanding targets and do not require a user implementation.

## Manual Install Quick Start

1. Use the provided defaults 
2. Build the library
3. Install the library
4. Include the library.

To complete steps 1-3 with one command try the following if your system supports `make`.

```zsh
make str_view [OPTIONAL/INSTALL/PATH]
```

This will use CMake and your default compiler to build and install the library in release mode. By default, this library does not touch your system paths and it is installed in the `install/` directory of this folder. This is best for testing the library out while pointing `cmake` to the install location. Then, deleting the `install/` folder deletes any trace of this library from your system.

Then, in your `CMakeLists.txt`:

```cmake
find_package(str_view HINTS "~/path/to/str_view-v[VERSION]/install")
```

If you want to simply write the following command in your `CMakeLists.txt`,

```cmake
find_package(str_view)
```

specify that this library shall be installed to a location CMake recognizes by default. For example, my preferred location is as follows:

```zsh
make str_view ~/.local
```

Then the installation looks like this.

```txt
.local
├── include
│   └── str_view
│       └── str_view.h
└── lib
    ├── cmake
    │   └── str_view
    │       ├── str_viewConfig.cmake
    │       ├── str_viewConfigVersion.cmake
    │       ├── str_viewTargets.cmake
    │       └── str_viewTargets-release.cmake
    └── libstr_view_release.a
```

Now to delete the library if needed, simply find all folders and files with the `*str_view*` string somewhere within them and delete. You can also check the `build/install_manifest.txt` to confirm the locations of any files installed with this library.

## Include the Library

Once CMake can find the package, link against it and include the `str_view.h` header.

The `CMakeLists.txt` file.

```cmake
# Optionally use str_view as a system library so that your tooling like clang-tidy ignores it.
get_target_property(str_view_SOURCE_DIR str_view SOURCE_DIR)
add_executable(my_exe my_exe.c)
target_include_directories(main SYSTEM PRIVATE ${str_view_SOURCE_DIR})
target_link_libraries(my_exe str_view::str_view)
```

The C code.

```.c
#include "str_view/str_view.h"
```

## Alternative Builds

You may wish to use a different compiler and toolchain than what your system default specifies. Review the `CMakePrests.json` file for different compilers.

```zsh
make gcc-str_view [OPTIONAL/INSTALL/PATH]
make install
```

Use Clang to compile the library.

```zsh
make clang-str_view [OPTIONAL/INSTALL/PATH]
make install
```

## Without Make

If your system does not support Makefiles or the `make` command here are the cmake commands one can run that will allow another generator such as `Ninja` to complete building and installation.

```zsh
# Configure the project cmake files. 
# Replace this preset with your own if you'd like.
cmake --preset=clang-release -DCMAKE_INSTALL_PREFIX=[DESIRED/INSTALL/LOCATION]
cmake --build build
cmake --build build --target install
```

## User Presets

If you do not like the default presets, create a `CMakeUserPresets.json` in this folder and place your preferred configuration in that file. Here is my preferred configuration to get you started.

```json
{
    "version": 3,
    "cmakeMinimumRequired": {
        "major": 3,
        "minor": 23,
        "patch": 0
    },
    "configurePresets": [
        {
            "name": "my-clang-debug",
            "inherits": ["default-debug"],
            "generator": "Ninja",
            "cacheVariables": {
                "CMAKE_C_COMPILER": "clang-22",
                "CMAKE_C_FLAGS": "$env{SV_WARNING_C_FLAGS} -g3 -glldb"
            }
        },
        {
            "name": "my-clang-release",
            "inherits": ["default-release"],
            "generator": "Ninja",
            "cacheVariables": {
                "CMAKE_C_COMPILER": "clang-22"
            }
        },
        {
            "name": "my-clang-memory-sanitize-debug",
            "inherits": ["clang-memory-sanitize-debug"],
            "generator": "Ninja",
            "cacheVariables": {
                "CMAKE_C_COMPILER": "clang-22"
            }
        },
        {
            "name": "my-clang-memory-sanitize-release",
            "inherits": ["clang-memory-sanitize-release"],
            "generator": "Ninja",
            "cacheVariables": {
                "CMAKE_C_COMPILER": "clang-22"
            }
        },
        {
            "name": "my-llvm-cov",
            "inherits": ["llvm-cov"],
            "generator": "Ninja",
            "cacheVariables": {
                "CMAKE_C_COMPILER": "clang-22",
                "CMAKE_C_FLAGS": "--coverage -g3 -glldb -Og $env{SV_WARNING_C_FLAGS}"
            }
        }
    ]
}
```
