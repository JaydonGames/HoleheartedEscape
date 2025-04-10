cmake_minimum_required(VERSION 3.11)
project("HoleheartedEscape")

add_compile_definitions(SDL_MAIN_HANDLED)

if (WIN32)
	set(GLEW_USE_STATIC_LIBS true)
	add_compile_definitions(GLEW_STATIC)
endif()

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_Image REQUIRED)

include_directories("include")
add_subdirectory("assets")
add_subdirectory("src/graphics")
add_subdirectory("src/sample")
