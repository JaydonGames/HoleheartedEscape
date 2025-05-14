file(GLOB SOURCES *.cpp)
add_library(Physics ${SOURCES})
target_include_directories(Physics PUBLIC "../../include/physics/")
set_property(TARGET Physics PROPERTY CXX_STANDARD 20)

