project(airplay-lib)

add_definitions(-DWIN32_LEAN_AND_MEAN -DHAS_DUMMY_RENDERER)

file(GLOB_RECURSE airplay_SRC ${PROJECT_SOURCE_DIR}/lib ${PROJECT_SOURCE_DIR}/llhttp ${PROJECT_SOURCE_DIR}/playfair *.c)
file(GLOB_RECURSE airplay_HEADER ${PROJECT_SOURCE_DIR}/lib ${PROJECT_SOURCE_DIR}/llhttp ${PROJECT_SOURCE_DIR}/playfair *.h)

set(airplay_main_SRC
	rpiplay.cpp
	../common/byteutils.c
)

set(airplay_extra_SOURCES
	airplay-server.rc
)
			 
set(airplay_extra_HEADERS
	resource.h
	rpiplay.h
	../common/byteutils.h
	../common/portable_endian.h
)
 
add_library(airplay-lib STATIC
	${airplay_SRC}
	${airplay_HEADER}
	${airplay_main_SRC}
	${airplay_extra_SOURCES}
	${airplay_extra_HEADERS})

target_link_libraries(airplay-lib PRIVATE
	libobs
	w32-pthreads)

target_include_directories(airplay-lib PRIVATE
	${PROJECT_SOURCE_DIR}/third-party/include
	${PROJECT_SOURCE_DIR}/lib
	${PROJECT_SOURCE_DIR}/../common
)
target_include_directories(airplay-lib PUBLIC .)

set_target_properties(airplay-lib PROPERTIES FOLDER "plugins/phone-camera/mobile-media-backend")
