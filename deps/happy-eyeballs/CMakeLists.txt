cmake_minimum_required(VERSION 3.22...3.25)

add_library(happy-eyeballs STATIC)

target_sources(
  happy-eyeballs
  PRIVATE happy-eyeballs.c
  PUBLIC happy-eyeballs.h)

target_include_directories(happy-eyeballs PUBLIC .)

target_link_libraries(happy-eyeballs PUBLIC libobs)

set_target_properties(happy-eyeballs PROPERTIES FOLDER deps)
