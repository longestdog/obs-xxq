project(beauty-filter)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(_bit_suffix "x64")
else()
	set(_bit_suffix "x86")
endif()

find_package(FFmpeg REQUIRED COMPONENTS avutil swscale)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
include_directories(${FFMPEG_INCLUDE_DIRS})

set(ANGLE_INCLUDE_PATH third/angle/include)
set(ANGLE_DYNAMIC_INCLUDE_PATH third/angle_dynamic/inc)
set(EFFECT_INCLUDE_PATH third/effect_sdk/tencent/include)

include_directories(
    ${ANGLE_INCLUDE_PATH}
    ${ANGLE_DYNAMIC_INCLUDE_PATH}
    ${EFFECT_INCLUDE_PATH}
)

add_definitions(-DUSE_ANGLE=1)
add_definitions(-DUSE_DYNAMIC_ANGLE=1)
add_definitions(-DEGL_EGL_PROTOTYPES=0)
add_definitions(-DGL_GLES_PROTOTYPES=0)

set(CMAKE_AUTOMOC TRUE)  
   	 
source_group("beauty" FILES ${BEAUTIFY_SRCS})
  
set(beauty-filter_HEADERS
	XMagicHandle.h
	beauty-handle.h
	misc.h
	)
  
set(beauty-filter_SOURCES
	XMagicHandle.cpp
	beauty-handle.cpp
	beauty-filter.cpp)

add_library(beauty-filter MODULE
	${beauty-filter_SOURCES}
	${beauty-filter_HEADERS})

target_include_directories(beauty-filter PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/third/effect_sdk/tencent/include
)

target_link_libraries(beauty-filter
	Qt5::Widgets
	Qt5::Network
	libobs
	debug ${CMAKE_CURRENT_SOURCE_DIR}/third/angle_dynamic/${_bit_suffix}/Debug/dynamic_angle.lib
	optimized ${CMAKE_CURRENT_SOURCE_DIR}/third/angle_dynamic/${_bit_suffix}/Release/dynamic_angle.lib
	debug ${CMAKE_CURRENT_SOURCE_DIR}/third/effect_sdk/tencent/libs/${_bit_suffix}/Debug/XmagicDll.lib
	optimized ${CMAKE_CURRENT_SOURCE_DIR}/third/effect_sdk/tencent/libs/${_bit_suffix}/Release/XmagicDll.lib
	)

set_target_properties(beauty-filter PROPERTIES FOLDER "plugins/beauty-filter")

install_obs_plugin(beauty-filter data)
