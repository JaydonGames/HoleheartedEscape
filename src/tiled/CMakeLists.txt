file(GLOB SOURCES *.cpp)
file(GLOB_RECURSE HEADERS RELATIVE ../../include/Tiled/ *.hpp)
add_library(Tiled ${SOURCES} ${HEADERS})
target_include_directories(Tiled PUBLIC "../../include/tiled/")
target_link_libraries(Tiled PUBLIC tinyxml2::tinyxml2)
set_property(TARGET Tiled PROPERTY CXX_STANDARD 20)

