cmake_minimum_required(VERSION 3.15)
project(idasourcemod)

# Requires IDASDK environment variable pointing to the IDA SDK repository root.
# When cloning HexRaysSA/ida-sdk, set IDASDK to the repo root.
# Headers are expected at $IDASDK/src/include/ and libs at $IDASDK/src/lib/.

if(NOT DEFINED ENV{IDASDK})
    message(FATAL_ERROR "IDASDK environment variable not set. "
        "Point it to the IDA SDK repository root (containing src/include/pro.h).")
endif()

set(IDASDK_ROOT "$ENV{IDASDK}")

# Locate include directory (supports both repo-root/src/include and direct include layouts)
if(EXISTS "${IDASDK_ROOT}/src/include/pro.h")
    set(IDASDK_INCLUDE "${IDASDK_ROOT}/src/include")
    set(IDASDK_LIBDIR  "${IDASDK_ROOT}/src/lib")
elseif(EXISTS "${IDASDK_ROOT}/include/pro.h")
    set(IDASDK_INCLUDE "${IDASDK_ROOT}/include")
    set(IDASDK_LIBDIR  "${IDASDK_ROOT}/lib")
else()
    message(FATAL_ERROR "Could not find pro.h under IDASDK=${IDASDK_ROOT}. "
        "Expected it at src/include/pro.h or include/pro.h.")
endif()

# Select the correct import library for the target platform
if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(IDASDK_LIB "${IDASDK_LIBDIR}/x64_win_vc_64/ida.lib")
    else()
        message(FATAL_ERROR "Only 64-bit builds are supported on Windows.")
    endif()
else()
    message(FATAL_ERROR "Only Windows builds are currently supported.")
endif()

if(NOT EXISTS "${IDASDK_LIB}")
    message(FATAL_ERROR "IDA import library not found: ${IDASDK_LIB}")
endif()

add_library(idasourcemod SHARED
    src/plugin.cpp
    src/netpropimporter.cpp
    src/structaligner.cpp
    src/isgoodsig.cpp
    src/jumptosig.cpp
    src/makesig.cpp
    src/makesigfromhere.cpp
    src/functionoffset.cpp
    src/pugixml/pugixml.cpp
)

target_include_directories(idasourcemod PRIVATE
    "${IDASDK_INCLUDE}"
    src
)

target_compile_definitions(idasourcemod PRIVATE
    __NT__
    __EA64__
)

target_link_libraries(idasourcemod PRIVATE "${IDASDK_LIB}")

# Export the PLUGIN symbol required by IDA to recognise the plugin
if(MSVC)
    target_link_options(idasourcemod PRIVATE /EXPORT:PLUGIN)
endif()

# Use the multi-threaded DLL runtime (/MD) as required by the IDA SDK
if(MSVC)
    set_target_properties(idasourcemod PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    )
endif()

# IDA plugins are plain DLLs – drop the "lib" prefix on non-Windows platforms
set_target_properties(idasourcemod PROPERTIES PREFIX "")
