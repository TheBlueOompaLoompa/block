#pragma once
#include <SDL2/SDL_ttf.h>
#include <glm/glm.hpp>

#include "imgui.h"
#include "helper.hpp"

struct UIData {
    bool f3;
    glm::vec3 pos;
    glm::vec2 look_dir;
};

void render_ui(UIData ui) {
    if(ui.f3) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("F3 Menu");
        ImGui::Text("Position X %f Y %f Z %f", V3FMT(ui.pos));
        ImGui::Text("Look dir X %f Y %f", V2FMT(ui.look_dir));
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}