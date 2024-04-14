#pragma once

#include <glm/glm.hpp>

#include <SDL2/SDL.h>

struct InputPreferences {
    SDL_Scancode forward = SDL_SCANCODE_W;
    SDL_Scancode backward = SDL_SCANCODE_S;
    SDL_Scancode left = SDL_SCANCODE_A;
    SDL_Scancode right = SDL_SCANCODE_D;

    SDL_Scancode jump = SDL_SCANCODE_SPACE;
    SDL_Scancode crouch = SDL_SCANCODE_LSHIFT;
};

struct Preferences {
    // UI
    bool fullscreen = false;
    int width = 1280;
    int height = 720;

    // Input
    InputPreferences input;
};