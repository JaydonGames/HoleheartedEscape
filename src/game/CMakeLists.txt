file(GLOB SOURCES *.cpp)
add_executable(HoleHeartedEscape ${SOURCES})
target_link_libraries(HoleHeartedEscape PUBLIC Graphics Assets Tiled Physics SDL2::SDL2 OpenGL::GL GLEW::GLEW SDL2_image::SDL2_image)
set_property(TARGET HoleHeartedEscape PROPERTY CXX_STANDARD 20)

