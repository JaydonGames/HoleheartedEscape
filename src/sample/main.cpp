#include <SDL.h>
#include <SDL_events.h>
#include <SDL_video.h>
#include "graphics/opengl.hpp"
#include "graphics/render.hpp"
#include "assets.hpp"

int main(){
    SDL_SetMainReady();
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Sample", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1080, 720,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    OpenGL::Context context{window};
    Render::BatchRenderer renderer;
    OpenGL::Texture tex1{Textures::tiles_png, Textures::tiles_png_length};
    OpenGL::Texture tex2{Textures::square_png, Textures::square_png_length};
    context.set_clear_color(.5,.5,.5);
    context.enable_vsync();
    renderer.set_canvas(1080, 720);

    for (;;) {
        context.clear();
        for (unsigned int x = 0; x < 4; ++x)
            for (unsigned int y = 0; y < 3; ++y)
                renderer.push({150+150*x, 150+150*y, 100, 100}, {x*tex1.width()/4, y*tex1.height()/3, tex1.width()/4, tex1.height()/3}, tex1);
        renderer.render();
        context.swap_buffer();
        
        SDL_Event e;
        SDL_PollEvent(&e);
        if (e.type == SDL_QUIT)
            break;
    }

    SDL_Quit();
}
