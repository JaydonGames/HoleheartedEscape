#include "assets.hpp"
#include "graphics/opengl.hpp"
#include "graphics/render.hpp"
#include <SDL2/SDL.h>
#include <SDL_events.h>
#include <SDL_video.h>
#include <iostream>

/*We can change the const, here for now for clarity*/
const int SCREEN_WIDTH = 1080;
const int SCREEN_HEIGHT = 720;

const int TILE_WIDTH = 64;
const int SCREEN_FPS = 60;
const double SCREEN_TICK_PER_FRAME = 1000 / SCREEN_FPS;

class Player {
public:
  static const int PLAYER_VEL = 300;

  Player();

  void input(SDL_Event &e);

  void move(double dt);

  Render::Rect get_rect();

private:
  Render::Rect m_rect;

  double m_dir_x, m_dir_y, m_gravity;

  bool is_jumping;
};

int main() {
  SDL_SetMainReady();
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = SDL_CreateWindow(
      "Sample", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
      SCREEN_HEIGHT,
      SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  OpenGL::Context context{window};
  Render::BatchRenderer renderer;
  OpenGL::Texture tex1{Textures::tiles_png, Textures::tiles_png_length};
  OpenGL::Texture tex_player{Textures::square_png, Textures::square_png_length};
  context.set_clear_color(.5, .5, .5);
  context.enable_vsync();
  renderer.set_canvas(SCREEN_WIDTH, SCREEN_HEIGHT);

  Player player;

  Uint64 LAST = 0;
  Uint64 CURR = SDL_GetPerformanceCounter();
  int frame_count = 0;
  double dt = 0;
  bool running = true;
  for (;;) {
    /*Immediately start timing frame to make it nanoseconds more accurate lol*/
    Uint64 cap_start = SDL_GetPerformanceCounter();

    /*Delta time*/
    CURR = SDL_GetPerformanceCounter();
    dt = ((CURR - LAST) / (double)SDL_GetPerformanceFrequency());
    if (std::isnan(dt)) {
      dt = 0;
    }
    LAST = CURR;

    std::cout << dt << '\n';

    /*Events*/
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        running = false;
        break;
      }
      player.input(e);
    }
    /*Do this or use a while loop? What is the diff between inf for loop and
     * while loop? -J*/
    if (!running) {
      break;
    }

    /*Updates*/
    player.move(dt);

    for (unsigned int x = 0; x < 4; ++x)
      for (unsigned int y = 0; y < 3; ++y)
        renderer.push({150 + 150 * x, 150 + 150 * y, 100, 100},
                      {x * TILE_WIDTH, y * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH},
                      tex1);

    renderer.push(player.get_rect(), {0, 0, TILE_WIDTH, TILE_WIDTH},
                  tex_player);

    /*Rendering*/
    context.clear();
    renderer.render();
    context.swap_buffer();

    ++frame_count;
    double frame_time_ms = (SDL_GetPerformanceCounter() - cap_start) * 1000.0 /
                           (double)SDL_GetPerformanceFrequency();
    if (frame_time_ms < SCREEN_TICK_PER_FRAME) {
      SDL_Delay((Uint32)(SCREEN_TICK_PER_FRAME - frame_time_ms));
    }

    /*SDL_Event e;*/
    /*SDL_PollEvent(&e);*/
    /*if (e.type == SDL_QUIT)*/
    /*  break;*/
  }

  SDL_Quit();
}

Player::Player() {
  m_dir_x = 0;
  m_dir_y = 0;

  m_rect.x = 0;
  m_rect.y = 0;
  m_rect.w = TILE_WIDTH;
  m_rect.h = TILE_WIDTH;

  m_gravity = 100;

  is_jumping = false;
}

void Player::input(SDL_Event &e) {
  if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
    switch (e.key.keysym.sym) {
    case SDLK_UP:
      if (!is_jumping) {
        m_dir_y = -20;
        is_jumping = true;
      }
      break;
    case SDLK_LEFT:
      m_dir_x -= 1;
      break;
    case SDLK_RIGHT:
      m_dir_x += 1;
      break;
    }
  } else if (e.type == SDL_KEYUP && e.key.repeat == 0) {
    switch (e.key.keysym.sym) {
    case SDLK_LEFT:
      m_dir_x += 1;
      break;
    case SDLK_RIGHT:
      m_dir_x -= 1;
      break;
    }
  }
}

void Player::move(double dt) {
  m_rect.x += m_dir_x * PLAYER_VEL * dt;

  m_dir_y += m_gravity * dt;
  if (m_dir_y > 30) {
    m_dir_y = 30;
  }
  m_rect.y += m_dir_y;
  if (m_rect.y > 720 - m_rect.h) {
    m_rect.y = 720 - m_rect.h;
    m_dir_y = 0;
    is_jumping = false;
  }
}

Render::Rect Player::get_rect() { return m_rect; }
