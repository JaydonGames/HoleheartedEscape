#include <SDL.h>
#include <SDL_events.h>
#include <SDL_video.h>
#include "graphics/opengl.hpp"
#include "graphics/render.hpp"
#include "tiled/map.hpp"
#include "assets.hpp"
#include "game/player.hpp"

constexpr int SCREEN_WIDTH = 1920;
constexpr int SCREEN_HEIGHT = 1080;

constexpr int SCREEN_FPS = 60;
constexpr double SCREEN_TICK_PER_FRAME = 1000 / SCREEN_FPS;

int main() {
    SDL_SetMainReady();
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Sample", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                          SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    OpenGL::Context context{window};
    Render::BatchRenderer renderer;
    Render::Camera camera{renderer};
    context.set_clear_color(.5, .5, .5);
    context.enable_vsync();
    constexpr Render::Vec2 canvas{SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4};
    context.set_canvas_size(SCREEN_WIDTH, SCREEN_HEIGHT);
    renderer.set_canvas(canvas.x, canvas.y);
    camera.set(canvas.x / 2, canvas.y / 2);

    Tiled::Map::register_texture("walls_tileset", Textures::walls_tileset);
    Tiled::Map::register_texture("ground_tileset", Textures::ground_tileset);
    Tiled::Map::register_tileset("walls_tileset", Tilesets::walls_tileset);
    Tiled::Map::register_tileset("ground_tileset", Tilesets::ground_tileset);
    Tiled::Map tilemap{Maps::map};

    Tiled::Map::register_texture("main_tileset", Textures::main_tileset);
    Tiled::Map::register_tileset("main_tileset", Tilesets::main_tileset);
    Tiled::Map test_map{Maps::test_map};

    OpenGL::Texture bg{Textures::background};
    OpenGL::Texture player_tex{Textures::Crystal};

    Player player;

    Uint64 LAST = 0;
    Uint64 CURR = SDL_GetPerformanceCounter();
    int frame_count = 0;
    double dt = 0;
    bool running = true;
    while (running) {
        Uint64 cap_start = SDL_GetPerformanceCounter();

        // Delta time
        CURR = SDL_GetPerformanceCounter();
        dt = ((CURR - LAST) / (double)SDL_GetPerformanceFrequency());
        if (std::isnan(dt)) {
            dt = 0;
        }
        LAST = CURR;

        // Events
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
                break;
            }
            player.input(e);
        }

        // Updating
        player.move(dt);

        renderer.push({0, 0}, {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}, bg, 0);
        for (auto &layer : test_map.layers) {
            for (int y = 0; y < layer.tiles.size(); ++y) {
                for (int x = 0; x < layer.tiles[y].size(); ++x) {
                    Tiled::Tile &tile = layer.tiles[y][x];
                    if (tile.empty)
                        continue;
                    Render::Rect coords = tile.get_coords();
                    renderer.push({x * coords.w, y * coords.h}, coords, tile.get_texture(), tile.get_flags());
                }
            }
        }
        Render::Rect player_rect = player.get_rect();
        renderer.push({player_rect.x, player_rect.y}, {0, 0, 16, 16}, player_tex, 0);

        // Rendering
        context.clear();
        renderer.render();
        context.swap_buffer();

        ++frame_count;
        double frame_time_ms =
            (SDL_GetPerformanceCounter() - cap_start) * 1000.0 / (double)SDL_GetPerformanceFrequency();
        if (frame_time_ms < SCREEN_TICK_PER_FRAME) {
            SDL_Delay((Uint32)(SCREEN_TICK_PER_FRAME - frame_time_ms));
        }
    }

    SDL_Quit();
}
