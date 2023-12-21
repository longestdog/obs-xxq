cmake_minimum_required(VERSION 3.14)
project(Algorithm CXX)

set(CMAKE_CXX_STANDARD 11)

if(NOT DEFINED PLATFORM OR "${PLATFORM}" STREQUAL "")
   set(PLATFORM x64)
endif()

if(ENABLE_PROFILE)
	add_definitions(-DENABLE_PROFILE)
endif()

add_definitions(-DALGORITHM_EXPORT=1)
add_definitions(-DTARGET_OS_WINDOWS=1)

if(MSVC12 OR MSVC14)	
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8 /MD /Zi")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /MD /Zi")
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG")
endif()

set(PROJECT_NAME Algorithm)

set(ALGORITHM_DIR  ${CMAKE_CURRENT_SOURCE_DIR})

set(EFFECT_INCLUDE_PATH ../third/effect_sdk/inc)
set(ALGORITHM_TASK_PATH AlgorithmTask)

FILE(GLOB_RECURSE ALGORITHM_TASK_FILES ${ALGORITHM_TASK_PATH}/*.h ${ALGORITHM_TASK_PATH}/*.cpp)

file(GLOB_RECURSE ALL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
                              ${CMAKE_CURRENT_SOURCE_DIR}/*.h
                              ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ALL_SOURCES} {ALGORITHM_TASK_FILES})

#SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../bin)

add_library(Algorithm STATIC ${ALL_SOURCES})

target_include_directories(Algorithm PRIVATE
  ${ALGORITHM_DIR}
  ${EFFECT_INCLUDE_PATH}
  ${ALGORITHM_TASK_PATH}
)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "plugins/beauty-filter")
