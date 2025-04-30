cmake_minimum_required(VERSION 3.12)

project(light_pcapng C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

option(LIGHT_PCAPNG_ZSTD "Build with ZSTD compression support" OFF)
option(LIGHT_PCAPNG_TESTS "Build tests" OFF)
set(LIGHT_PCAPNG_BUILD_TYPE "STATIC" CACHE STRING "Build type: STATIC, OBJECT, or SHARED")

# Validate input
if(NOT LIGHT_PCAPNG_BUILD_TYPE IN_LIST ALLOWED_BUILD_TYPES)
  set(ALLOWED_BUILD_TYPES STATIC OBJECT SHARED)
  if(NOT LIGHT_PCAPNG_BUILD_TYPE IN_LIST ALLOWED_BUILD_TYPES)
    message(FATAL_ERROR "Invalid value for LIGHT_PCAPNG_BUILD_TYPE: ${LIGHT_PCAPNG_BUILD_TYPE}. Allowed: ${ALLOWED_BUILD_TYPES}")
  endif()
endif()

set(LIGHT_PCAPNG_SOURCES
  src/light_advanced.c
  src/light_alloc.c
  src/light_compression.c
  src/light_internal.c
  src/light_io.c
  src/light_manipulate.c
  src/light_null_compression.c
  src/light_option.c
  src/light_pcapng_cont.c
  src/light_pcapng_ext.c
  src/light_pcapng.c
  src/light_platform.c
  src/light_zstd_compression.c
)

if(LIGHT_PCAPNG_BUILD_TYPE STREQUAL "STATIC")
  add_library(light_pcapng STATIC ${LIGHT_PCAPNG_SOURCES})
elseif(LIGHT_PCAPNG_BUILD_TYPE STREQUAL "OBJECT")
  add_library(light_pcapng OBJECT ${LIGHT_PCAPNG_SOURCES})
elseif(LIGHT_PCAPNG_BUILD_TYPE STREQUAL "SHARED")
  add_library(light_pcapng SHARED ${LIGHT_PCAPNG_SOURCES})
  set_property(TARGET light_pcapng PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

target_compile_definitions(light_pcapng PUBLIC -DUNIVERSAL)

target_include_directories(
  light_pcapng
  PUBLIC $<INSTALL_INTERFACE:include> $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

if(LIGHT_PCAPNG_ZSTD)
  find_package(ZSTD)
  if(NOT ZSTD_FOUND)
      message(FATAL_ERROR "ZSTD required but not found!")
  elseif(ZSTD_VERSION_STRING VERSION_LESS "1.4.0")
      message(FATAL_ERROR "ZSTD >= 1.4.0 required!")
  endif()
  # TODO: is it needed?
  add_definitions(-DUSE_Z_STD)

  # Linking with objects required CMake 3.12
  cmake_minimum_required(VERSION 3.12)
  target_link_libraries(light_pcapng PUBLIC ZSTD::ZSTD)
  target_compile_definitions(light_pcapng PUBLIC -DUSE_Z_STD)
endif()

if(LIGHT_PCAPNG_TESTS)
  add_executable(test_read.test src/tests/test_read.c)
  target_link_libraries(test_read.test light_pcapng)

  add_executable(test_read_write.test src/tests/test_read_write.c)
  target_link_libraries(test_read_write.test light_pcapng)

  add_executable(test_mem.test src/tests/test_mem.c)
  target_link_libraries(test_mem.test light_pcapng)

  add_executable(test_histogram.test src/tests/test_histogram.c)
  target_link_libraries(test_histogram.test light_pcapng)

  add_executable(test_subcapture.test src/tests/test_subcapture.c)
  target_link_libraries(test_subcapture.test light_pcapng)

  add_executable(test_feature.test src/tests/test_feature.c)
  target_link_libraries(test_feature.test light_pcapng)

  add_executable(test_flow.test src/tests/test_flow.c)
  target_link_libraries(test_flow.test light_pcapng)

  #add_executable(test_feature_advanced.test src/tests/test_feature_advanced.c)
  #target_link_libraries(test_feature_advanced.test light_pcapng dl)

  add_executable(test_read_packets.test src/tests/test_read_packets.c)
  target_link_libraries(test_read_packets.test light_pcapng)

  add_executable(test_read_write_packets.test src/tests/test_read_write_packets.c)
  target_link_libraries(test_read_write_packets.test light_pcapng)

  #add_executable(test_split.test src/tests/test_split.c)
  #target_link_libraries(test_split.test light_pcapng)
endif()
