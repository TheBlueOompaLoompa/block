#pragma once

#include <glm/glm.hpp>
#include <SDL2/SDL.h>

#include "util.hpp"

const char* PREFS_FILE = "prefs.bin";

struct InputPreferences {
    SDL_Scancode forward = SDL_SCANCODE_W;
    SDL_Scancode backward = SDL_SCANCODE_S;
    SDL_Scancode left = SDL_SCANCODE_A;
    SDL_Scancode right = SDL_SCANCODE_D;

    SDL_Scancode jump = SDL_SCANCODE_SPACE;
    SDL_Scancode sprint = SDL_SCANCODE_LCTRL;
    SDL_Scancode crouch = SDL_SCANCODE_LSHIFT;
};

struct GraphicsPreferences {
    float fov = 90.0f;
};

struct Preferences {
    // UI
    bool fullscreen = false;
    int width = 1280;
    int height = 720;

    InputPreferences input;
    GraphicsPreferences graphics;

    void save() {
        save_data(PREFS_FILE, this, sizeof(Preferences));
    }

    void load() {
        if(!std::filesystem::exists(PREFS_FILE)) return;
        load_data(PREFS_FILE, this, sizeof(Preferences));
    }
};