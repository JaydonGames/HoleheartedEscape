file(GLOB SOURCES *.cpp)
add_library(Tiled ${SOURCES})
target_include_directories(Tiled PUBLIC "../../include/tiled/")
target_link_libraries(Tiled PUBLIC tinyxml2::tinyxml2)
set_property(TARGET Tiled PROPERTY CXX_STANDARD 20)

