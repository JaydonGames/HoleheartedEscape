#include "game/player.hpp"
#include <SDL_events.h>
#include "graphics/types.hpp"
#include <cstdlib>
#include <iostream>
#include <utility>

// Player::Player(Render::Vector2D pos) : Square(pos, true) {
//     m_is_jumping = false;
// };
//
// void Player::input(SDL_Event &e) {
//     if (e.type == SDL_KEYDOWN) {
//         switch (e.key.keys m.sym) {
//             case SDLK_UP:
//                 if (!m_is_jumping) {
//                     acceleration.y = -PLAYER_JUMP_FORCE;
//                     m_is_jumping = true;
//                 }
//                 break;
//             case SDLK_RIGHT:
//                 acceleration.x = 10;
//                 break;
//             case SDLK_LEFT:
//                 acceleration.x = -10;
//                 break;
//         }
//     }
//     if (e.type == SDL_KEYUP) {
//         switch (e.key.keysym.sym) {
//             case SDLK_RIGHT:
//                 acceleration.x = 0;
//                 break;
//             case SDLK_LEFT:
//                 acceleration.x = 0;
//                 break;
//         }
//     }
// }
