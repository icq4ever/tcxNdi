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

message(STATUS "[tcxNdi]   include : ${TCX_NDI_INCLUDE_DIR}")
message(STATUS "[tcxNdi]   library : ${TCX_NDI_LIBRARY}")

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

message(STATUS "[tcxNdi] Addon loaded")
