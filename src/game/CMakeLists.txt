file(GLOB SOURCES *.cpp)
file(GLOB_RECURSE HEADERS RELATIVE ../../include/game *.hpp)
add_executable(HoleHeartedEscape ${SOURCES} ${HEADERS})
target_link_libraries(HoleHeartedEscape PUBLIC Graphics Assets Tiled Physics SDL2::SDL2 OpenGL::GL GLEW::GLEW SDL2_image::SDL2_image)
set_property(TARGET HoleHeartedEscape PROPERTY CXX_STANDARD 20)

