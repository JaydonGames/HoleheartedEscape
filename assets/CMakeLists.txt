include(../embed.cmake)
add_library(Assets STATIC ../include/assets.hpp)

file(GLOB SHADERS */*.vert */*.frag)
foreach(SHADER ${SHADERS})
    embed(Assets "${SHADER}" "Shaders" "const char" WITH_EXT)
endforeach()

file(GLOB TEXTURES */*.png */*.jpg */*.jpeg */*.tiff */*.bmp)
foreach(TEXTURE ${TEXTURES})
    embed(Assets "${TEXTURE}" "Textures" "const unsigned char" LENGTH)
endforeach()

file(GLOB MAPS */*.tmx)
foreach(MAP ${MAPS})
    embed(Assets "${MAP}" "Maps" "const char" NULL_DELIM)
endforeach()

file(GLOB TILESETS */*.tsx)
foreach(TILESET ${TILESETS})
    embed(Assets "${TILESET}" "Tilesets" "const char" NULL_DELIM)
endforeach()
