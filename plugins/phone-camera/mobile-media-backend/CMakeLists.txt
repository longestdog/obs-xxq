project(mobile-media-backend)

find_package(PLIST REQUIRED)
find_package(LibUSB REQUIRED)
find_package(LibEVENT REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(fdk-aac REQUIRED)
find_package(Libimobiledevice REQUIRED)
find_package(Libusbmuxd REQUIRED)

find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

set(CMAKE_AUTOMOC TRUE)

include_directories(SYSTEM ${PLIST_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/ios/screenmirror)
include_directories(${PROJECT_SOURCE_DIR}/ios/usbmuxd)
include_directories(${PROJECT_SOURCE_DIR}/ios/airplay/third-party/include)
include_directories(${PROJECT_SOURCE_DIR}/ios/airplay/lib)

add_definitions("-DHAVE_LIBIMOBILEDEVICE")
add_definitions("-DHAVE_ENUM_IDEVICE_CONNECTION_TYPE")
add_definitions("-DWIN32_LEAN_AND_MEAN")
add_definitions("-DWIN32_LEAN_AND_MEAN -DHAS_DUMMY_RENDERER")

file(GLOB usbmuxd_SRC ${PROJECT_SOURCE_DIR}/ios/usbmuxd/*.c)
file(GLOB usbmuxd_HEADER ${PROJECT_SOURCE_DIR}/ios/usbmuxd/*.h)

file(GLOB screenmirror_SRC ${PROJECT_SOURCE_DIR}/ios/screenmirror/*.c)
file(GLOB screenmirror_HEADER ${PROJECT_SOURCE_DIR}/ios/screenmirror/*.h)

file(GLOB_RECURSE iOS_airplay_SRC ${PROJECT_SOURCE_DIR}/ios/airplay/lib ${PROJECT_SOURCE_DIR}/ios/airplay/llhttp ${PROJECT_SOURCE_DIR}/ios/airplay/playfair *.c)
file(GLOB_RECURSE iOS_airplay_HEADER ${PROJECT_SOURCE_DIR}/ios/airplay/lib ${PROJECT_SOURCE_DIR}/ios/airplay/llhttp ${PROJECT_SOURCE_DIR}/ios/airplay/playfair *.h)

set(iOS_airplay_main_SRC
	ios/airplay/rpiplay.cpp
)

set(iOS_airplay_extra_SOURCES
	ios/airplay/airplay-server.rc
)
			 
set(iOS_airplay_extra_HEADERS
	ios/airplay/resource.h
)
 

set(mobile-media-backend_HEADERS
	../common.h
	../xxq-tcp.h
	../ipc.h
	application.h
	usb-helper.h
	c-util.h
	driver-helper.h
	media-task.h
	media-source.h)

set(mobile-media-backend_SOURCES 
	main.cpp
	../xxq-tcp.cpp
	../ipc.c
	application.cpp
	usb-helper.cpp
	c-util.cpp
	driver-helper.cpp
	media-task.cpp
	media-source.cpp)

add_executable(mobile-media-backend
	${mobile-media-backend_SOURCES}
	${mobile-media-backend_HEADERS}
	${usbmuxd_SRC}
	${usbmuxd_HEADER}
	${screenmirror_SRC}
	${screenmirror_HEADER}
	${iOS_airplay_SRC}
	${iOS_airplay_HEADER}
	${iOS_airplay_main_SRC} 
	${iOS_airplay_extra_SOURCES}
	${iOS_airplay_extra_HEADERS})

foreach(lib ${PLIST_LIBRARIES} ${OpenSSL_LIBRARIES} ${FDKAAC_LIBRARIES} ${IMOBILEDEVICE_LIBRARIES} ${LIBUSBMUXD_LIBRARIES} ${LIB_USB_LIBRARIES} ${LIB_EVENT_LIBRARIES} ${FDKAAC_LIBRARIES})
    list (APPEND RELEASE_LINK_LIST optimized ${lib})
endforeach()

foreach(lib ${PLIST_DEBUG_LIBRARIES} ${OpenSSL_DEBUG_LIBRARIES} ${FDKAAC_DEBUG_LIBRARIES} ${IMOBILEDEVICE_DEBUG_LIBRARIES} ${LIBUSBMUXD_DEBUG_LIBRARIES} ${LIB_USB_DEBUG_LIBRARIES} ${LIB_EVENT_DEBUG_LIBRARIES} ${FDKAAC_DEBUG_LIBRARIES})
    list (APPEND DEBUG_LINK_LIST debug ${lib})
endforeach()

target_link_libraries(mobile-media-backend
	${RELEASE_LINK_LIST}
	${DEBUG_LINK_LIST}
	${FFMPEG_LIBRARIES}
	libobs
	w32-pthreads
	Setupapi
	Qt5::Gui
	Qt5::Widgets
	Qt5::Network
	Winusb
	Ws2_32
	)

set_target_properties(mobile-media-backend PROPERTIES FOLDER "plugins/phone-camera")

install_obs_core(mobile-media-backend)
