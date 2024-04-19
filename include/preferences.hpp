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

struct GraphicsPreferences {
    float fov = 90.0f;
};

#define PREFS_FILE "prefs.bin"
struct Preferences {
    // UI
    bool fullscreen = false;
    int width = 1280;
    int height = 720;

    InputPreferences input;
    GraphicsPreferences graphics;

    static void save(Preferences *self) {
        auto file = fopen(PREFS_FILE, "w");

        fwrite(self, sizeof(Preferences), 1, file);
        fclose(file);
    }

    static void load(Preferences *self) {
        if(!std::filesystem::exists(PREFS_FILE)) return;
        auto file = fopen(PREFS_FILE, "r");

        fread(self, sizeof(Preferences), 1, file);
        fclose(file);
    }
};