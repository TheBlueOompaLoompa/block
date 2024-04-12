#pragma once
#include <SDL2/SDL_ttf.h>
#include <glm/glm.hpp>

#include "imgui.h"
#include "helper.hpp"

struct UIData {
    bool f3;

    glm::vec3 pos;
    glm::vec3 rot;
    glm::vec2 look_dir;
    
    float fps = 60.0f;
};

void render_ui(UIData ui) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if(ui.f3) {
        ImGui::Begin("F3 Menu", &ui.f3, 
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoFocusOnAppearing|
            ImGuiWindowFlags_NoInputs);
        ImGui::Text("Position X %f Y %f Z %f", V3FMT(ui.pos));
        glm::vec3 newrot = ui.rot/glm::vec3(M_PI/180.0f);
        ImGui::Text("Rotation X %f Y %f Z %f", V3FMT(newrot));
        ImGui::Text("Look dir X %f Y %f", V2FMT(ui.look_dir));
        ImGui::Text("FPS %f", ui.fps);
        ImGui::End();
    }


    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}