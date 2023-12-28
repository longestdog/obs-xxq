project(rtmp-lib)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/third-party/ZLToolKit/src)
include_directories(${PROJECT_SOURCE_DIR}/third-party/include)

set(VS_FALGS "/wd4819")

add_definitions("-D_HAS_STD_BYTE=0")

file(GLOB_RECURSE rtmp_SRC ${PROJECT_SOURCE_DIR}/*.c ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE rtmp_HEADER ${PROJECT_SOURCE_DIR}/*.h ${PROJECT_SOURCE_DIR}/*.hpp)

add_library(rtmp-lib STATIC
	${rtmp_SRC}
	${rtmp_HEADER})

target_include_directories(rtmp-lib PUBLIC src)

target_link_libraries(rtmp-lib PRIVATE libobs)

set_target_properties(rtmp-lib PROPERTIES FOLDER "plugins/phone-camera/mobile-media-backend")
