cmake_minimum_required(VERSION 3.21)

project(AudioDiagramTools VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ADT_BUILD_TESTS "Build Audio Diagram Tools smoke tests" ON)

include(FetchContent)

if(POLICY CMP0169)
  cmake_policy(SET CMP0169 OLD)
endif()

FetchContent_Declare(cgif
  GIT_REPOSITORY https://github.com/dloebl/cgif.git
  GIT_TAG f1272ac2b0150e6f699163d70421b0347b72ec8e
)

FetchContent_GetProperties(cgif)
if(NOT cgif_POPULATED)
  FetchContent_Populate(cgif)
endif()

add_library(cgif STATIC
  "${cgif_SOURCE_DIR}/src/cgif.c"
  "${cgif_SOURCE_DIR}/src/cgif_raw.c"
  "${cgif_SOURCE_DIR}/src/cgif_rgb.c"
)

target_include_directories(cgif
  PUBLIC
    "${cgif_SOURCE_DIR}/inc"
)

set_target_properties(cgif PROPERTIES
  C_STANDARD 99
  C_STANDARD_REQUIRED ON
  C_EXTENSIONS OFF
)

if(MSVC)
  target_compile_options(cgif PRIVATE /W0)
else()
  target_compile_options(cgif PRIVATE -w)
endif()

set(AUDIO_DIAGRAM_VISAGE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/build/_vendor/visage"
    CACHE PATH "Path to a local Visage checkout")

if(NOT IS_ABSOLUTE "${AUDIO_DIAGRAM_VISAGE_DIR}")
  get_filename_component(AUDIO_DIAGRAM_VISAGE_DIR
                         "${AUDIO_DIAGRAM_VISAGE_DIR}"
                         ABSOLUTE
                         BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

if(NOT EXISTS "${AUDIO_DIAGRAM_VISAGE_DIR}/CMakeLists.txt")
  message(FATAL_ERROR
    "Visage was not found at '${AUDIO_DIAGRAM_VISAGE_DIR}'. "
    "Clone Visage there or configure with -DAUDIO_DIAGRAM_VISAGE_DIR=<path>.")
endif()

set(VISAGE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(VISAGE_BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory("${AUDIO_DIAGRAM_VISAGE_DIR}" "${CMAKE_BINARY_DIR}/_deps/visage")

add_library(audio_diagram_tools STATIC
  src/audio_diagram_tools/gif_exporter.cpp
  src/audio_diagram_tools/png_export.cpp
  src/audio_diagram_tools/render_types.cpp
  src/audio_diagram_tools/sampled_waveform_scene.cpp
  src/audio_diagram_tools/style_lab_scene.cpp
  src/audio_diagram_tools/waveform_scene.cpp
)

add_library(adt::audio_diagram_tools ALIAS audio_diagram_tools)

target_include_directories(audio_diagram_tools
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
  PRIVATE
    "${CMAKE_BINARY_DIR}/_deps/visage/visage_graphics/VisageEmbeddedFonts_generated"
    "${AUDIO_DIAGRAM_VISAGE_DIR}/visage_file_embed"
)

target_link_libraries(audio_diagram_tools
  PUBLIC
    visage
  PRIVATE
    cgif
)

target_compile_features(audio_diagram_tools PUBLIC cxx_std_17)

if(MSVC)
  target_compile_options(audio_diagram_tools PRIVATE /W4 /permissive-)
else()
  target_compile_options(audio_diagram_tools PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(adt_render
  examples/adt_render.cpp
)

target_link_libraries(adt_render
  PRIVATE
    adt::audio_diagram_tools
)

add_executable(adt_encode_gif
  examples/adt_encode_gif.cpp
)

target_link_libraries(adt_encode_gif
  PRIVATE
    adt::audio_diagram_tools
)

add_executable(adt_style_lab
  examples/adt_style_lab.cpp
)

target_link_libraries(adt_style_lab
  PRIVATE
    adt::audio_diagram_tools
)

add_executable(adt_eight_sample_waveform
  examples/adt_eight_sample_waveform.cpp
)

target_link_libraries(adt_eight_sample_waveform
  PRIVATE
    adt::audio_diagram_tools
)

if(ADT_BUILD_TESTS)
  enable_testing()

  add_executable(adt_smoke_tests
    tests/smoke_render.cpp
  )

  target_link_libraries(adt_smoke_tests
    PRIVATE
      adt::audio_diagram_tools
  )

  add_test(NAME adt_smoke_tests COMMAND adt_smoke_tests)

  add_executable(adt_gif_smoke_tests
    tests/smoke_gif.cpp
  )

  target_link_libraries(adt_gif_smoke_tests
    PRIVATE
      adt::audio_diagram_tools
  )

  add_test(NAME adt_gif_smoke_tests COMMAND adt_gif_smoke_tests)
endif()
