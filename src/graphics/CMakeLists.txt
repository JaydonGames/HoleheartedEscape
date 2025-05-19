file(GLOB SOURCES *.cpp)
file(GLOB_RECURSE HEADERS RELATIVE ../../include/graphics/ *.hpp)
add_library(Graphics ${SOURCES} ${HEADERS})
target_include_directories(Graphics PUBLIC "../../include/graphics/")
target_link_libraries(Graphics PUBLIC Assets SDL2::SDL2 OpenGL::GL GLEW::GLEW SDL2_image::SDL2_image)
set_property(TARGET Graphics PROPERTY CXX_STANDARD 20)

