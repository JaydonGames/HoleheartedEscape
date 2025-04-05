file(GLOB SOURCES *.cpp)
add_executable(Sample ${SOURCES})
target_link_libraries(Sample PUBLIC Graphics Assets SDL2::SDL2 OpenGL::GL GLEW::GLEW SDL2_image::SDL2_image)
set_property(TARGET Sample PROPERTY CXX_STANDARD 20)

