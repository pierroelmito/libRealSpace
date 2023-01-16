cmake_minimum_required(VERSION 3.21)

project(NeoStrikeCommander)
set(CMAKE_CXX_STANDARD 20)
add_compile_definitions(USE_SHADER_PIPELINE=1)
include_directories(src src/sc src/ext)

add_custom_command(
  PRE_BUILD
  OUTPUT ${CMAKE_SOURCE_DIR}/src/shaders/shaders.h
  COMMAND sokol-shdc --input ${CMAKE_SOURCE_DIR}/src/shaders/shaders.glsl --output ${CMAKE_SOURCE_DIR}/src/shaders/shaders.h --slang glsl330
  DEPENDS ${CMAKE_SOURCE_DIR}/src/shaders/shaders.glsl
)

file(
  GLOB_RECURSE
  sourcelist
  src/*.cpp
)

add_executable(
  sc
  src/shaders/shaders.h
  ${sourcelist}
)

target_link_libraries(sc -lGL -lglfw -lpthread -lasound)

