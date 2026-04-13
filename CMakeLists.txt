cmake_minimum_required(VERSION 3.27)
project(idasourcemod)

# Requires IDASDK environment variable pointing to the IDA SDK directory.
# The SDK cmake support (ida-cmake) is expected at $IDASDK/cmake/bootstrap.cmake.
# When cloning HexRaysSA/ida-sdk with submodules, set IDASDK to the repo root or src/.

if(NOT DEFINED ENV{IDASDK})
    message(FATAL_ERROR "IDASDK environment variable not set. "
        "Point it to the IDA SDK directory (containing include/pro.h or src/include/pro.h).")
endif()

# Locate bootstrap.cmake (handles both repo-root and src/ layouts)
set(_bootstrap_paths
    "$ENV{IDASDK}/cmake/bootstrap.cmake"
    "$ENV{IDASDK}/src/cmake/bootstrap.cmake"
)
foreach(_p ${_bootstrap_paths})
    if(EXISTS "${_p}")
        include("${_p}")
        break()
    endif()
endforeach()

if(NOT COMMAND ida_add_plugin)
    message(FATAL_ERROR "Could not find bootstrap.cmake under IDASDK=$ENV{IDASDK}. "
        "Make sure the SDK was cloned with --recurse-submodules.")
endif()

find_package(idasdk REQUIRED)

ida_add_plugin(idasourcemod
    SOURCES
        src/plugin.cpp
        src/netpropimporter.cpp
        src/structaligner.cpp
        src/isgoodsig.cpp
        src/jumptosig.cpp
        src/makesig.cpp
        src/makesigfromhere.cpp
        src/functionoffset.cpp
        src/pugixml/pugixml.cpp
    INCLUDES
        src
)
