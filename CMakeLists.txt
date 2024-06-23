cmake_minimum_required(VERSION 3.10)

project(psp-space)

add_executable(${PROJECT_NAME} main.cpp)

include(FindPkgConfig)
pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2_IMAGE REQUIRED SDL2_image)
pkg_search_module(SDL2_MIXER REQUIRED SDL2_mixer)
pkg_search_module(SDL2_TTF REQUIRED SDL2_ttf)

target_link_libraries(${PROJECT_NAME} PRIVATE
	${SDL2_LIBRARIES}
	${SDL2_IMAGE_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
	${SDL2_TTF_LIBRARIES}
	SDL2main
)

if(PLATFORM_PSP)
	create_pbp_file(
		TARGET ${PROJECT_NAME}
		ICON_PATH NULL
		BACKGROUND_PATH NULL
		PREVIEW_PATH NULL
		TITLE ${PROJECT_NAME}
	)
elseif(VITA)
	set(VITA_TITLEID "INPUT0001") #title length == 9

	include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

	vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})
	vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
		VERSION "01.00"
		NAME ${PROJECT_NAME}
		FILE platform/vita/sce_sys sce_sys
		FILE res/fonts/square_sans_serif_7.ttf square_sans_serif_7.ttf
		FILE res/sounds/laser.ogg laser.wav
		FILE res/sounds/explosion.ogg explosion.wav
		FILE res/sprites/alien_1.png alien_1.png 
		FILE res/sprites/alien_2.png alien_2.png 
		FILE res/sprites/alien_3.png alien_3.png 
		FILE res/sprites/mystery.png mystery.png 
		FILE res/sprites/spaceship.png spaceship.png 
		FILE res/sprites/structure.png structure.png 
	)
endif()

file(COPY res/fonts/square_sans_serif_7.ttf DESTINATION ${CMAKE_BINARY_DIR})
file(COPY res/sounds/laser.wav DESTINATION ${CMAKE_BINARY_DIR})
file(COPY res/sounds/explosion.wav DESTINATION ${CMAKE_BINARY_DIR})
file(COPY res/sprites/alien_1.png DESTINATION ${CMAKE_BINARY_DIR})
file(COPY res/sprites/alien_2.png DESTINATION ${CMAKE_BINARY_DIR})
file(COPY res/sprites/alien_3.png DESTINATION ${CMAKE_BINARY_DIR})
file(COPY res/sprites/mystery.png DESTINATION ${CMAKE_BINARY_DIR})
file(COPY res/sprites/spaceship.png DESTINATION ${CMAKE_BINARY_DIR})
file(COPY res/sprites/structure.png DESTINATION ${CMAKE_BINARY_DIR})

