file(GLOB SOURCES *.cpp)
file(GLOB_RECURSE HEADERS RELATIVE ../../include/physics/ *.hpp)
add_library(Physics ${SOURCES} ${HEADERS})
target_include_directories(Physics PUBLIC "../../include/physics/")
set_property(TARGET Physics PROPERTY CXX_STANDARD 20)

