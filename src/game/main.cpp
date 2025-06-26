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
    Math::Vec2<int> start_size{SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};

    SDL_SetMainReady();
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Sample", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, start_size.x,
                                          start_size.y, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    OpenGL::Context::debug = true;
    OpenGL::Context context{window};
    // Context::wireframe(true)
    Render::BatchRenderer renderer;
    Render::SimpleRenderer fbo_renderer;
    Render::Camera screen_camera, camera;
    Render::Canvas canvas{SCREEN_WIDTH, SCREEN_HEIGHT}, screen{start_size.x, start_size.y};
    camera.set(canvas.x / 2, canvas.y / 2, 0.25f);
    context.set_clear_color(0, 0, 0);
    context.enable_vsync();

    OpenGL::Texture color_msa{OpenGL::Texture::tex2d_msa, SCREEN_WIDTH, SCREEN_HEIGHT, 4};
    OpenGL::Texture color{OpenGL::Texture::tex2d, SCREEN_WIDTH, SCREEN_HEIGHT};
    canvas.fbo.create();
    canvas.fbo.attach(0, color_msa);
    OpenGL::Framebuffer fbo;
    fbo.create();
    fbo.attach(0, color);

    Math::Vec2 light{int(0.25 * canvas.x / 2), int(0.25 * canvas.y / 2)};

    Render::TextureGroup textures;
    size_t bg = textures.push_back(Textures::background);
    size_t player_tex = textures.push_back(Textures::Crystal);
    size_t object_tex = textures.push_back(Textures::objects);
    size_t main_tileset = textures.push_back(Textures::main_tileset);
    textures.finalize();

    Tiled::World world;
    world.register_texture("main_tileset", main_tileset);
    world.register_tileset("main_tileset", Tilesets::main_tileset);
    Tiled::Map test_map{world, Maps::test_map};

    PhysicsWorld engine;

    Player player{Math::Vec2<float>(64, 160), 5.0f};
    engine.add_square(&player);

    Square object{Math::Vec2<float>(80, 224), 3.0f, 16.0f};
    engine.add_square(&object);

    Tiled::Layer collision_layer = test_map[0];
    std::deque<Square> collision_tiles;
    for (int y = 0; y < collision_layer.tiles.size(); ++y) {
        for (int x = 0; x < collision_layer.tiles[y].size(); ++x) {
            if (!collision_layer.tiles[y][x].tile)
                continue;
            collision_tiles.emplace_back(Math::Vec2<float>(x * 16, y * 16), -1.0f, 16.0f, true);
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
        // std::cout << "X: " << player.get_particles()[0]->acceleration.x << '\n';
        // std::cout << "y: " << player.get_particles()[0]->acceleration.y << "\n\n";

        // Updating
        engine.update(dt);

        renderer.push({0, 0}, {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}, bg, 0, Render::NoSelfShadows);
        for (auto &layer : test_map.layers) {
            for (int y = 0; y < layer.tiles.size(); ++y) {
                for (int x = 0; x < layer.tiles[y].size(); ++x) {
                    Tiled::Tile *tile = layer.tiles[y][x].tile;
                    if (!tile)
                        continue;
                    renderer.push({x * tile->coords.w, y * tile->coords.h}, tile->coords, tile->tex, 0,
                                  layer.tiles[y][x].flags);
                }
            }
        }

        Math::Vec2<float> player_curr_position = player.get_curr_position();
        renderer.push({int(player_curr_position.x), int(player_curr_position.y)}, {0, 0, 16, 16}, player_tex,
                      player.get_angle(), 0);
        std::cout << "Angle: " << player.get_angle() << "\n";

        // std::array<VerletParticle *, 4> parts = player.get_particles();
        // std::cout << "0x: " << parts[0]->curr_position.x << '\n';
        // std::cout << "0y: " << parts[0]->curr_position.y << '\n';
        // std::cout << "1x: " << parts[1]->curr_position.x << '\n';
        // std::cout << "1y: " << parts[1]->curr_position.y << '\n';
        // std::cout << "2x: " << parts[2]->curr_position.x << '\n';
        // std::cout << "2y: " << parts[2]->curr_position.y << '\n';
        // std::cout << "3x: " << parts[3]->curr_position.x << '\n';
        // std::cout << "3y: " << parts[3]->curr_position.y << '\n';
        // std::cout << '\n';

        Math::Vec2<float> object_curr_position = object.get_curr_position();
        renderer.push({int(object_curr_position.x), int(object_curr_position.y)}, {0, 0, 16, 16}, object_tex,
                      object.get_angle(), 0);

        // Light
        const uint8_t *keystate = SDL_GetKeyboardState(NULL);
        if (keystate[SDL_SCANCODE_W])
            --light.y;
        if (keystate[SDL_SCANCODE_S])
            ++light.y;
        if (keystate[SDL_SCANCODE_A])
            --light.x;
        if (keystate[SDL_SCANCODE_D])
            ++light.x;

        renderer.push(light, Math::Color{0.977f, 0.848f, 0.7f}, canvas.y / 2, 0.5f, 0.9f);

        // Rendering
        camera.use();
        renderer.clear(canvas);
        renderer.render(canvas, textures);

        screen_camera.use();
        float zoom = std::max(float(canvas.x) / screen.x, float(canvas.y) / screen.y);
        int offset_x = (screen.x * zoom - canvas.x) / 2, offset_y = (screen.y * zoom - canvas.y) / 2;
        screen_camera.set(screen.x / 2, screen.y / 2, zoom);

        fbo.blit(canvas.fbo, {0, 0, int(canvas.x), int(canvas.y)}, {0, 0, int(canvas.x), int(canvas.y)},
                 OpenGL::Texture::linear);
        fbo_renderer.clear(screen);
        fbo_renderer.render(screen, {offset_x, offset_y}, {0, 0, int(canvas.x), int(canvas.y)}, color);
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
