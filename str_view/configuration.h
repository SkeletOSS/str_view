/** @file
@brief The String View Configuration Header.

This file supports the library's ability to operate on freestanding targets.
For full download and install instructions using a user configuration see
INSTALL.md.

The str_view library uses the following functions or macros that must be
supported by the user on freestanding targets.

Traditionally included via `<string.h>`:

- `memmove()`
- `memcmp()`
- `strlen()`
- `strnlen()`

To provide these functions, the user may create a header. For example,
`my_str_view_configuration.h`. In this header the user has two options: provide
the listed functions directly or include their versions of the headers that
provide the needed functionality. It is common for freestanding environments to
provide their own `<string.h>` that implement these functions.

Then, define `SV_USER_CONFIGURATION="my_sv_configuration.h"` at some point in
the build steps. Either provide it in a CMakeLists.txt file, in a
CMakePresets.json file, or as a command line argument. The library is then fully
configured for a freestanding environment. See INSTALL.md for the full download
and build instructions.

Any other headers such as `<stdint.h>`, `<stddef.h>`, etc. that are included
directly in source code now, or will be in the future, are those provided by the
C standard on freestanding targets. */
#ifndef SV_HOSTED_VS_FREESTANDING_CONFIGURATION_H
#define SV_HOSTED_VS_FREESTANDING_CONFIGURATION_H

#ifdef SV_USER_CONFIGURATION_HEADER
#    include SV_USER_CONFIGURATION_HEADER /* IWYU pragma: export */
#else
#    include <string.h> /* IWYU pragma: export */
#endif

#endif /* SV_HOSTED_VS_FREESTANDING_CONFIGURATION_H */
