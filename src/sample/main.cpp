#include <SDL.h>
#include <SDL_events.h>
#include <SDL_video.h>
#include "graphics/opengl.hpp"
#include "graphics/render.hpp"
#include "assets.hpp"
#include "tiled/map.hpp"

int main(){
    SDL_SetMainReady();
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Sample", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1080, 720,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    OpenGL::Context context{window};
    Render::BatchRenderer renderer;
    Render::Camera camera{renderer};
    context.set_clear_color(.5,.5,.5);
    context.enable_vsync();
    renderer.set_canvas(1080, 720);

    Tiled::Map::register_texture("walls_tileset", Textures::walls_tileset);
    Tiled::Map::register_texture("ground_tileset", Textures::ground_tileset);
    Tiled::Map::register_tileset("walls_tileset", Tilesets::walls_tileset);
    Tiled::Map::register_tileset("ground_tileset", Tilesets::ground_tileset);
    Tiled::Map tilemap{Maps::map};

    for (;;) {
        context.clear();

        constexpr int tile_size = 15;
        for (auto& layer : tilemap.layers){
            for (int y = 0; y < layer.tiles.size(); ++y){
                for (int x = 0; x < layer.tiles[y].size(); ++x){
                    Tiled::Tile& tile = layer.tiles[y][x];
                    if (tile.empty)
                        continue;
                    renderer.push(
                            Render::Rect{x*tile_size, y*tile_size, tile_size, tile_size}, 
                            tile.get_coords(), tile.get_texture(), tile.get_flags()
                    );
                }
            }
        }

        renderer.render();
        context.swap_buffer();
        
        SDL_Event e;
        SDL_PollEvent(&e);
        if (e.type == SDL_QUIT)
            break;
    }

    SDL_Quit();
}
