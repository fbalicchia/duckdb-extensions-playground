cmake_minimum_required(VERSION 2.8.12)

if(POLICY CMP0026)
  cmake_policy(SET CMP0026 NEW)
endif()

if(POLICY CMP0015)
  cmake_policy(SET CMP0015 NEW)
endif()

if(POLICY CMP0042)
  cmake_policy(SET CMP0042 NEW)
endif()

if(POLICY CMP0051)
  cmake_policy(SET CMP0051 NEW)
endif()

if(CMAKE_VERSION VERSION_LESS "3.1")
  add_compile_options(-std=c++11)
else()
  set(CMAKE_CXX_STANDARD 11)
endif()

set(TARGET_NAME first_extensions)

project(${TARGET_NAME} C CXX)

option(
  OSX_BUILD_UNIVERSAL
  "Build both architectures on OSX and create a single binary containing both."
  FALSE)
if(OSX_BUILD_UNIVERSAL)
  if(NOT APPLE)
    error("This only makes sense on OSX")
  endif()
  set(CMAKE_OSX_ARCHITECTURES
      "x86_64;arm64"
      CACHE STRING "Build architectures for Mac OS X" FORCE)
endif()

find_package(Threads REQUIRED)

set(H3_SOURCE_DIR "${CMAKE_SOURCE_DIR}/h3/src/h3lib")
set(H3_BINARY_DIR "${CMAKE_SOURCE_DIR}/h3/build/h3/lib")

set(SRCS
"${H3_SOURCE_DIR}/lib/algos.c"
"${H3_SOURCE_DIR}/lib/coordijk.c"
"${H3_SOURCE_DIR}/lib/bbox.c"
"${H3_SOURCE_DIR}/lib/polygon.c"
"${H3_SOURCE_DIR}/lib/h3Index.c"
"${H3_SOURCE_DIR}/lib/vec2d.c"
"${H3_SOURCE_DIR}/lib/vec3d.c"
"${H3_SOURCE_DIR}/lib/vertex.c"
"${H3_SOURCE_DIR}/lib/linkedGeo.c"
"${H3_SOURCE_DIR}/lib/localij.c"
"${H3_SOURCE_DIR}/lib/latLng.c"
"${H3_SOURCE_DIR}/lib/directedEdge.c"
"${H3_SOURCE_DIR}/lib/mathExtensions.c"
"${H3_SOURCE_DIR}/lib/iterators.c"
"${H3_SOURCE_DIR}/lib/vertexGraph.c"
"${H3_SOURCE_DIR}/lib/faceijk.c"
"${H3_SOURCE_DIR}/lib/baseCells.c"
)

configure_file("${H3_SOURCE_DIR}/include/h3api.h.in" "${H3_BINARY_DIR}/include/h3api.h")
add_library(_h3 ${SRCS})
target_include_directories(_h3 SYSTEM PUBLIC "${H3_SOURCE_DIR}/include")
target_include_directories(_h3 SYSTEM PUBLIC "${H3_BINARY_DIR}/include")
target_compile_definitions(_h3 PRIVATE H3_HAVE_VLA)





# skip building extensions on mingw
if(WIN32 AND NOT MSVC)
  return()
endif()

include_directories(${DUCKDB_INCLUDE_FOLDER})

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  add_compile_options(-Wall -pedantic -fsanitize=address -fno-sanitize=vptr)
endif()

link_directories(${DUCKDB_LIBRARY_FOLDER})

add_library(${TARGET_NAME} SHARED first_extensions.cpp)
set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "")
target_link_libraries(${TARGET_NAME} _h3 Threads::Threads)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  target_link_libraries(${TARGET_NAME} -fsanitize=address)
endif()

set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".duckdb_extension")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS
                                                  "-undefined dynamic_lookup")
endif()

if(WIN32)
  target_link_libraries(${TARGET_NAME} duckdb ${DUCKDB_EXTRA_LINK_FLAGS})
endif()

if(MSVC)
  set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG
                                                  ${CMAKE_BINARY_DIR})
  set_target_properties(
    ${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE
                              ${CMAKE_BINARY_DIR})
endif()

if(WIN32)
  target_link_libraries(${TARGET_NAME} wsock32 ws2_32 wldap32 secur32)
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG
                                                  "/DELAYLOAD:duckdb.dll")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG
      "${CMAKE_EXE_LINKER_FLAGS_DEBUG}  /DELAYLOAD:duckdb.dll")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE
                                                  "/DELAYLOAD:duckdb.dll")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE
      "${CMAKE_EXE_LINKER_FLAGS_RELEASE}  /DELAYLOAD:duckdb.dll")
  # This is only strictly required in non-Visual-Studio builds like Ninja:
  target_link_libraries(${TARGET_NAME} delayimp)
endif()