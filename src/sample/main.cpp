#include "assets.hpp"
#include "graphics/opengl.hpp"
#include "graphics/render.hpp"
#include "tiled/map.hpp"
#include <SDL.h>
#include <SDL_events.h>
#include <SDL_video.h>

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

int main() {
  SDL_SetMainReady();
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = SDL_CreateWindow(
      "Sample", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
      SCREEN_HEIGHT,
      SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

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

  OpenGL::Texture bg{Textures::background};
  OpenGL::Texture player{Textures::Crystal};

  for (;;) {
    context.clear();

    /*for (auto &layer : tilemap.layers) {*/
    /*  for (int y = 0; y < layer.tiles.size(); ++y) {*/
    /*    for (int x = 0; x < layer.tiles[y].size(); ++x) {*/
    /*      Tiled::Tile &tile = layer.tiles[y][x];*/
    /*      if (tile.empty)*/
    /*        continue;*/
    /*      Render::Rect coords = tile.get_coords();*/
    /*      renderer.push({x * coords.w, y * coords.h}, coords,*/
    /*                    tile.get_texture(), tile.get_flags());*/
    /*    }*/
    /*  }*/
    /*}*/
    renderer.push({0, 0}, {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}, bg, 0);
    renderer.push({16, 16}, {0, 0, 16, 16}, player, 0);

    renderer.render();
    context.swap_buffer();

    SDL_Event e;
    SDL_PollEvent(&e);
    if (e.type == SDL_QUIT)
      break;
  }

  SDL_Quit();
}
