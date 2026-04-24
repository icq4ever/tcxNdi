cmake_minimum_required(VERSION 3.20)
message(STATUS "[tcxNdi] Loading addon...")

# =============================================================================
# Locate the NDI SDK
# =============================================================================
# The SDK is proprietary and cannot be bundled. Users must install it
# separately (see README.md). We probe the standard install locations
# per platform; users can override with NDI_SDK_ROOT.

set(_TCX_NDI_HINTS)
if(DEFINED NDI_SDK_ROOT)
    list(APPEND _TCX_NDI_HINTS "${NDI_SDK_ROOT}")
endif()
if(DEFINED ENV{NDI_SDK_ROOT})
    list(APPEND _TCX_NDI_HINTS "$ENV{NDI_SDK_ROOT}")
endif()

if(APPLE)
    list(APPEND _TCX_NDI_HINTS
        "/Library/NDI SDK for Apple"
        "/Library/NDI Advanced SDK for Apple"
    )
elseif(WIN32)
    # NDI SDK installs include the version in the folder name
    file(GLOB _TCX_NDI_WIN_DIRS
        "C:/Program Files/NDI/NDI*SDK"
        "C:/Program Files/NDI/NDI*Advanced*SDK"
    )
    list(APPEND _TCX_NDI_HINTS ${_TCX_NDI_WIN_DIRS})

    # The runtime DLL is shipped as a separate "NDI Runtime" installer on
    # recent SDKs (6.x) and lands under a versioned subfolder (e.g. v6).
    file(GLOB _TCX_NDI_WIN_RUNTIME_DIRS
        "C:/Program Files/NDI/NDI*Runtime/v*"
        "C:/Program Files/NDI/NDI*Runtime"
    )
    list(APPEND _TCX_NDI_HINTS ${_TCX_NDI_WIN_RUNTIME_DIRS})
else()
    list(APPEND _TCX_NDI_HINTS
        "/usr"
        "/usr/local"
        "/opt/NDI"
    )
endif()

find_path(TCX_NDI_INCLUDE_DIR
    NAMES Processing.NDI.Lib.h
    HINTS ${_TCX_NDI_HINTS}
    PATH_SUFFIXES include include/ndi
)

if(APPLE)
    find_library(TCX_NDI_LIBRARY
        NAMES ndi
        HINTS ${_TCX_NDI_HINTS}
        PATH_SUFFIXES lib/macOS lib
    )
elseif(WIN32)
    find_library(TCX_NDI_LIBRARY
        NAMES Processing.NDI.Lib.x64
        HINTS ${_TCX_NDI_HINTS}
        PATH_SUFFIXES Lib/x64
    )
    # Look in both the SDK layout (Bin/x64) and the standalone Runtime
    # installer layout (the versioned folder itself contains the DLL).
    find_file(TCX_NDI_RUNTIME_DLL
        NAMES Processing.NDI.Lib.x64.dll
        HINTS ${_TCX_NDI_HINTS}
        PATH_SUFFIXES Bin/x64 .
    )
else()
    find_library(TCX_NDI_LIBRARY
        NAMES ndi
        HINTS ${_TCX_NDI_HINTS}
        PATH_SUFFIXES lib lib/x86_64-linux-gnu lib/aarch64-linux-gnu
    )
endif()

if(NOT TCX_NDI_INCLUDE_DIR OR NOT TCX_NDI_LIBRARY)
    message(FATAL_ERROR
        "[tcxNdi] NDI SDK not found.\n"
        "  Install the SDK from https://ndi.video/sdk/ and either let CMake\n"
        "  find it in the default location, or set NDI_SDK_ROOT to point at\n"
        "  the SDK install root.\n"
        "  See addons/tcxNdi/README.md for details.")
endif()

if(WIN32 AND NOT TCX_NDI_RUNTIME_DLL)
    message(FATAL_ERROR
        "[tcxNdi] NDI runtime DLL not found.\n"
        "  Expected Processing.NDI.Lib.x64.dll under the SDK's Bin/x64\n"
        "  directory, or under 'C:/Program Files/NDI/NDI * Runtime/v*/'.\n"
        "  Install the NDI Runtime (bundled with NDI Tools, or a separate\n"
        "  installer on recent SDKs) or set NDI_SDK_ROOT to point at a tree\n"
        "  that contains the DLL.\n"
        "  See addons/tcxNdi/README.md for details.")
endif()

message(STATUS "[tcxNdi]   include : ${TCX_NDI_INCLUDE_DIR}")
message(STATUS "[tcxNdi]   library : ${TCX_NDI_LIBRARY}")
if(WIN32)
    message(STATUS "[tcxNdi]   runtime : ${TCX_NDI_RUNTIME_DLL}")
endif()

# =============================================================================
# Library target
# =============================================================================
file(GLOB TCX_NDI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB TCX_NDI_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

add_library(tcxNdi STATIC ${TCX_NDI_SOURCES} ${TCX_NDI_HEADERS})

target_include_directories(tcxNdi
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${TCX_NDI_INCLUDE_DIR}"
)

target_link_libraries(tcxNdi
    PUBLIC
        TrussC
        "${TCX_NDI_LIBRARY}"
)

if(WIN32)
    # TrussC Windows apps place the executable in <project>/bin, so keep the
    # NDI runtime next to the final .exe without changing core build logic.
    add_custom_command(TARGET tcxNdi POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/bin"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${TCX_NDI_RUNTIME_DLL}"
            "${CMAKE_SOURCE_DIR}/bin/Processing.NDI.Lib.x64.dll"
        COMMENT "[tcxNdi] Copying NDI runtime DLL to ${CMAKE_SOURCE_DIR}/bin"
    )
endif()

message(STATUS "[tcxNdi] Addon loaded")
