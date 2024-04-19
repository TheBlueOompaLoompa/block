#pragma once
#include <SDL2/SDL_ttf.h>
#include <glm/glm.hpp>

#include "imgui.h"
#include "helper.hpp"
#include "preferences.hpp"
#include "config.hpp"

struct UIData {
    bool f3;
    bool esc;
    bool quit = false;

    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec2 look_dir;

    glm::vec3 hit_pos;
    
    double fps = 60.0;
    float time = 0.0f;
};

const char* CenterText(const char* text) {
    float alignment = 0.5f;
    ImGuiStyle& style = ImGui::GetStyle();

    float size = ImGui::CalcTextSize(text).x + style.FramePadding.x * 2.0f;
    float avail = ImGui::GetContentRegionAvail().x;

    float off = (avail - size) * alignment;
    if (off > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
    return text;
}

// Returns true if prefs changed
bool render_ui(UIData* ui, Preferences *prefs) {
    bool changed = false;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if(ui->f3) {
        ImGui::Begin("F3 Menu", &ui->f3, 
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoFocusOnAppearing|
            ImGuiWindowFlags_NoInputs);
        ImGui::SetWindowFontScale(1.5f);
        ImGui::Text("Position\nX %f\nY %f\nZ %f", V3FMT(ui->pos));
        ImGui::Text("Hit Pos\nX %f\nY %f\nZ %f", V3FMT(ui->hit_pos));
        ImGui::Text("Velocity X %f Y %f Z %f", V3FMT(ui->vel));
        ImGui::Text("Look dir X %f Y %f", V2FMT(ui->look_dir));
        ImGui::Text("Window size %i %i", prefs->width, prefs->height);
        ImGui::Text("FPS %f", ui->fps);
        ImGui::Text("Time %f", ui->time);
        ImGui::End();
    }

    if(ui->esc) {
        ImGui::Begin("Escape Menu", &ui->esc,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
        ImGui::SetWindowFontScale(1.5f);

        ImGui::SetWindowPos(ImVec2 { 0.0, 0.0 });
        ImGui::SetWindowSize(ImVec2 { (float)prefs->width, (float)prefs->height });

        if(ImGui::Button("Back to Game")) {
            ui->esc = false;
        }

        if(ImGui::Button("Toggle Fullscreen")) {
            prefs->fullscreen = !prefs->fullscreen;
            changed = true;
        }

        ImGui::SliderFloat("Field of View", &prefs->graphics.fov, 20.0f, 120.0f);

        if(ImGui::Button("Quit")) {
            ui->quit = true;
        }

        ImGui::End();
    }

    ImGui::Begin("How to Play", NULL,
    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoFocusOnAppearing|
            ImGuiWindowFlags_NoInputs);
    ImGui::SetWindowFontScale(2.0f);
    ImGui::Text("Use W A S D to move");
    ImGui::Text("Use mouse to look");
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return changed;
}