project(obs-qsv11)

find_package(VPL REQUIRED)

set(obs-qsv11_SOURCES
	common_directx11.cpp
	common_utils.cpp
	common_utils_windows.cpp
	QSV_Encoder.cpp
	QSV_Encoder_Internal.cpp
	obs-qsv11.c
	obs-qsv11-plugin-main.c)

set(obs-qsv11_HEADERS
	bits/linux_defs.h
	bits/windows_defs.h
	common_directx11.h
	common_utils.h
	QSV_Encoder.h
	QSV_Encoder_Internal.h)

add_library(obs-qsv11 MODULE
	${obs-qsv11_SOURCES}
	${obs-qsv11_HEADERS}
	)
target_link_libraries(obs-qsv11
	libobs
	d3d11
	dxva2
	dxgi
	dxguid
	w32-pthreads
	VPL::VPL
	)

target_compile_definitions(obs-qsv11 PRIVATE DX11_D3D)
set_target_properties(obs-qsv11 PROPERTIES FOLDER "plugins/obs-qsv11")

source_group("obs-qsv11\\Source Files" FILES ${obs-qsv11_SOURCES})
source_group("obs-qsv11\\Header Files" FILES ${obs-qsv11_HEADERS})

install_obs_plugin_with_data(obs-qsv11 data)

if(WIN32)
	add_subdirectory(obs-qsv-test)
endif()
