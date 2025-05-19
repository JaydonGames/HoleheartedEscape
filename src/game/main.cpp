#include <SDL.h>
#include <SDL_events.h>
#include <SDL_keycode.h>
#include <SDL_video.h>
#include "graphics/opengl.hpp"
#include "graphics/render.hpp"
#include "tiled/map.hpp"
#include "assets.hpp"
#include "game/player.hpp"
#include "game/object.hpp"
#include "physics/physicsworld.hpp"
#include "physics/square.hpp"
#include <array>
#include <iostream>

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

    Tiled::Map::register_texture("main_tileset", Textures::main_tileset);
    Tiled::Map::register_tileset("main_tileset", Tilesets::main_tileset);
    Tiled::Map test_map{Maps::test_map};

    OpenGL::Texture bg{Textures::background};
    OpenGL::Texture player_tex{Textures::Crystal};
    OpenGL::Texture object_tex{Textures::objects};

    PhysicsWorld engine;
    Square player{Render::Vector2D(64, 160), 16.0f};
    int player_index = engine.add_square(std::move(player));
    Tiled::Layer collision_layer = test_map.layers[0];
    for (int y = 0; y < collision_layer.tiles.size(); ++y) {
        for (int x = 0; x < collision_layer.tiles[y].size(); ++x) {
            Tiled::Tile &tile = collision_layer.tiles[y][x];
            if (tile.empty)
                continue;
            Square collision_tile{Render::Vector2D(x * 16, y * 16), 16.0f, true};
            engine.add_square(std::move(collision_tile));
        }
    }
    ObjectTest collision_object{80, 224, collision_layer};

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
        if (dt > 100) {
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
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP: {
                        std::cout << "test\n";
                        std::array<VerletParticle *, 4> particles = engine.get_square(player_index).get_particles();
                        for (VerletParticle *p : particles) {
                            p->accelerate(Render::Vector2D(0, -1000.0f));
                        }
                        break;
                    }
                    case SDLK_RIGHT: {
                        std::array<VerletParticle *, 4> particles = engine.get_square(player_index).get_particles();
                        for (VerletParticle *p : particles) {
                            p->accelerate(Render::Vector2D(200.0f, 0));
                        }
                        break;
                    }
                    case SDLK_LEFT: {
                        std::array<VerletParticle *, 4> particles = engine.get_square(player_index).get_particles();
                        for (VerletParticle *p : particles) {
                            p->accelerate(Render::Vector2D(-200.0f, 0));
                        }
                        break;
                    }
                }
            }
        }

        // Updating
        engine.update(dt);
        collision_object.update(dt);

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

        Render::Vector2D player_curr_position = engine.get_square(player_index).get_curr_position();
        renderer.push({player_curr_position.x, player_curr_position.y}, {0, 0, 16, 16}, player_tex, 0);
        std::array<VerletParticle *, 4> parts = engine.get_square(player_index).get_particles();
        std::cout << "0x: " << parts[0]->curr_position.x << '\n';
        std::cout << "0y: " << parts[0]->curr_position.y << '\n';
        std::cout << "1x: " << parts[1]->curr_position.x << '\n';
        std::cout << "1y: " << parts[1]->curr_position.y << '\n';
        std::cout << "2x: " << parts[2]->curr_position.x << '\n';
        std::cout << "2y: " << parts[2]->curr_position.y << '\n';
        std::cout << "3x: " << parts[3]->curr_position.x << '\n';
        std::cout << "3y: " << parts[3]->curr_position.y << '\n';
        std::cout << '\n';

        Render::Rect test_object_rect = collision_object.get_rect();
        renderer.push({test_object_rect.x, test_object_rect.y}, {0, 0, 16, 16}, object_tex, 0);

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
