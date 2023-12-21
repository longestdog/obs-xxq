cmake_minimum_required(VERSION 3.14)
project(EffectManager CXX)

set(CMAKE_CXX_STANDARD 11)

if(WIN32)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()


if(NOT DEFINED PLATFORM OR "${PLATFORM}" STREQUAL "")
   set(PLATFORM x64)
endif()

if(ENABLE_PROFILE)
	add_definitions(-DENABLE_PROFILE)
endif()

if(MSVC12 OR MSVC14)	
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8 /MD /Zi")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /MD /Zi")
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG")
endif()

set(USE_ANGLE ON)
set(USE_DYNAMIC_ANGLE ON)

set(PROJECT_NAME EffectManager)


if(USE_ANGLE)
    add_definitions(-DUSE_ANGLE=1)
	if(USE_DYNAMIC_ANGLE)
        add_definitions(-DUSE_DYNAMIC_ANGLE=1)
        add_definitions(-DEGL_EGL_PROTOTYPES=0)
        add_definitions(-DGL_GLES_PROTOTYPES=0)
    endif()
else()
    set(USE_DYNAMIC_ANGLE OFF)
endif()

add_definitions(-DEFFECT_MANAGER_EXPORT=1)
add_definitions(-DTARGET_OS_WINDOWS=1)



set(EFFECT_MANAGER_DIR  ${CMAKE_CURRENT_SOURCE_DIR})


# third_include
set(ANGLE_INCLUDE_PATH ../third/angle/include)
set(ANGLE_DYNAMIC_INCLUDE_PATH ../third/angle_dynamic/inc)
set(EFFECT_INCLUDE_PATH ../third/effect_sdk/inc)
set(UTILS_PATH utils)


FILE(GLOB_RECURSE UTILS_FILES ${UTILS_PATH}/*.h ${UTILS_PATH}/*.cpp)

file(GLOB_RECURSE ALL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
                              ${CMAKE_CURRENT_SOURCE_DIR}/*.h
                              ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ALL_SOURCES})


#SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../bin)


add_library(EffectManager STATIC ${ALL_SOURCES} ${UTILS_FILES})

target_include_directories(EffectManager PRIVATE
  ${ANGLE_INCLUDE_PATH}
  ${ANGLE_DYNAMIC_INCLUDE_PATH}
  ${EFFECT_INCLUDE_PATH}
)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "plugins/beauty-filter")
