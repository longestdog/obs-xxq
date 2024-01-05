project(android-adb-lib)

if(APPLE)
	set(_bit_suffix "")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(_bit_suffix "64bit/")
else()
	set(_bit_suffix "32bit/")
endif()

file(GLOB_RECURSE android_adb_lib_SRC ${PROJECT_SOURCE_DIR} *.c *.cpp util/*.c util/*.cpp)
file(GLOB_RECURSE android_adb_lib_HEADER ${PROJECT_SOURCE_DIR} *.h *.hpp util/*.h util/*.hpp) 

add_library(android-adb-lib STATIC
	${android_adb_lib_SRC} 
	${android_adb_lib_HEADER})

target_link_libraries(android-adb-lib
	libobs
	Ws2_32 
	w32-pthreads)
target_include_directories(android-adb-lib PRIVATE .)
set_target_properties(android-adb-lib PROPERTIES FOLDER "plugins/phone-camera/mobile-media-backend")

install_obs_core(android-adb-lib)

function(install_dependency target)
add_custom_command(TARGET android-adb-lib POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy
			"${PROJECT_SOURCE_DIR}/deps/${target}"
			"${OBS_OUTPUT_DIR}/$<CONFIGURATION>/bin/${_bit_suffix}${target}"
		VERBATIM)
add_custom_command(TARGET android-adb-lib POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy
			"${PROJECT_SOURCE_DIR}/deps/${target}"
			"$<TARGET_FILE_DIR:android-adb-lib>/${target}"
		VERBATIM)
endfunction()

install_dependency("adb.exe")
install_dependency("AdbWinApi.dll")
install_dependency("AdbWinUsbApi.dll")
install_dependency("scrcpy-server")
