cmake_minimum_required(VERSION 3.22...3.25)

add_library(opts-parser STATIC)

target_sources(
  opts-parser
  PRIVATE opts-parser.c
  PUBLIC opts-parser.h)

target_include_directories(opts-parser PUBLIC .)

target_link_libraries(opts-parser PUBLIC libobs)

set_target_properties(opts-parser PROPERTIES FOLDER deps)
