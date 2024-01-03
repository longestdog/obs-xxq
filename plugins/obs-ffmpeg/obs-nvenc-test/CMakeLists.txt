project(obs-nvenc-test)

include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/libobs)

add_executable(obs-nvenc-test)
target_sources(obs-nvenc-test PRIVATE obs-nvenc-test.c)
target_link_libraries(obs-nvenc-test d3d11 dxgi dxguid)

set_target_properties(obs-nvenc-test PROPERTIES FOLDER "plugins/obs-ffmpeg")

install_obs_core(obs-nvenc-test)
