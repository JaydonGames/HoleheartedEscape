#include <SDL.h>
#include <SDL_events.h>
#include <SDL_keycode.h>
#include <SDL_video.h>
#include "graphics/opengl.hpp"
#include "graphics/render.hpp"
#include "tiled/map.hpp"
#include "assets.hpp"
#include "game/object.hpp"
#include "game/player.hpp"
#include "physics/physicsworld.hpp"
#include "physics/square.hpp"
#include <array>
#include <deque>
#include <iostream>

constexpr int SCREEN_WIDTH = 1920;
constexpr int SCREEN_HEIGHT = 1080;

constexpr int SCREEN_FPS = 60;
constexpr double SCREEN_TICK_PER_FRAME = 1000 / SCREEN_FPS;

// TODO: Modularize this (Aydon help)
int main() {
    constexpr Render::Vec2 start_size{SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};

    SDL_SetMainReady();
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Sample", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, start_size.x,
                                          start_size.y, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    OpenGL::Context context{window};
    Render::BatchRenderer renderer;
    Render::Camera screen_camera, camera;
    Render::Canvas canvas{SCREEN_WIDTH, SCREEN_HEIGHT}, screen{start_size.x, start_size.y};
    camera.set(canvas.x / 2, canvas.y / 2, 0.25f);
    context.set_clear_color(0, 0, 0);
    context.enable_vsync();

    OpenGL::Framebuffer& fbo = canvas.fbo;
    OpenGL::Texture color{OpenGL::Texture::tex2d, SCREEN_WIDTH, SCREEN_HEIGHT, OpenGL::format<8>(OpenGL::Format::RGBA)};
    fbo.create();
    fbo.attach(0, color);

    Tiled::Map::register_texture("main_tileset", Textures::main_tileset);
    Tiled::Map::register_tileset("main_tileset", Tilesets::main_tileset);
    Tiled::Map test_map{Maps::test_map};

    OpenGL::Texture bg{Textures::background};
    OpenGL::Texture player_tex{Textures::Crystal};
    OpenGL::Texture object_tex{Textures::objects};

    PhysicsWorld engine;

    Player player{Render::Vector2D(64, 160)};
    engine.add_square(&player);

    Square object{Render::Vector2D(80, 224), 16.0f};
    engine.add_square(&object);

    Tiled::Layer collision_layer = test_map.layers[0];
    std::deque<Square> collision_tiles;
    for (int y = 0; y < collision_layer.tiles.size(); ++y) {
        for (int x = 0; x < collision_layer.tiles[y].size(); ++x) {
            Tiled::Tile &tile = collision_layer.tiles[y][x];
            if (tile.empty)
                continue;
            collision_tiles.emplace_back(Render::Vector2D(x * 16, y * 16), 16.0f, true);
            engine.add_square(&collision_tiles.back());
        }
    }

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
            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
                screen.x = e.window.data1;
                screen.y = e.window.data2;
            }
        }
        player.input();
        std::cout << "X: " << player.get_particles()[0]->acceleration.x << '\n';
        std::cout << "y: " << player.get_particles()[0]->acceleration.y << "\n\n";

        // Updating
        engine.update(dt);

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

        Render::Vector2D player_curr_position = player.get_curr_position();
        renderer.push({int(player_curr_position.x), int(player_curr_position.y)}, {0, 0, 16, 16}, player_tex, 0);
        // std::array<VerletParticle *, 4> parts = engine.get_square(player_index).get_particles();
        // std::cout << "0x: " << parts[0]->curr_position.x << '\n';
        // std::cout << "0y: " << parts[0]->curr_position.y << '\n';
        // std::cout << "1x: " << parts[1]->curr_position.x << '\n';
        // std::cout << "1y: " << parts[1]->curr_position.y << '\n';
        // std::cout << "2x: " << parts[2]->curr_position.x << '\n';
        // std::cout << "2y: " << parts[2]->curr_position.y << '\n';
        // std::cout << "3x: " << parts[3]->curr_position.x << '\n';
        // std::cout << "3y: " << parts[3]->curr_position.y << '\n';
        // std::cout << '\n';

        Render::Vector2D object_curr_position = object.get_curr_position();
        renderer.push({int(object_curr_position.x), int(object_curr_position.y)}, {0, 0, 16, 16}, object_tex, 0);

        // Rendering
        camera.bind();
        renderer.clear(canvas);
        renderer.render(canvas);

        screen_camera.bind();
        float zoom = std::max(float(canvas.x) / screen.x, float(canvas.y) / screen.y);
        screen_camera.set(screen.x / 2, screen.y / 2, zoom);
        renderer.push({int((screen.x * zoom - canvas.x) / 2), int((screen.y * zoom - canvas.y) / 2)},
                      {0, 0, int(canvas.x), int(canvas.y)}, color, Render::FlipY);
        renderer.clear(screen);
        renderer.render(screen);

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
