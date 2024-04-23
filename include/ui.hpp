#pragma once
#include <SDL2/SDL_ttf.h>
#include <glm/glm.hpp>

#include "imgui.h"
#include "helper.hpp"
#include "preferences.hpp"
#include "config.hpp"

struct UIResources {
    GLuint* atlas;
};

struct UIState {
    int save_combo;
    bool create_menu;
    char new_save_name[50];
    int new_world_seed;
    std::filesystem::path save_folder;
};

struct UIData {
    bool f3 = false;
    bool esc = false;
    bool main_menu = true;
    bool quit = false;

    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec2 look_dir;

    glm::vec3 hit_pos;
    
    double fps = 60.0;
    float time = 0.0f;

    const char* save_folders;

    void (*setup_func)();
    void (*save_func)();

    UIState state;
    UIResources res;
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

int fake_callback(ImGuiInputTextCallbackData* data) {
    return 0;
}

void reset_ui(UIData* ui) {
    ui->esc = false;
    ui->f3 = false;
    ui->state.create_menu = false;
    ui->state.new_world_seed = rand();
    ui->state.save_combo = 0;
    ui->main_menu = true;
}

// Returns true if prefs changed
bool render_ui(UIData* ui, Preferences *prefs) {
    bool changed = false;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if(!ui->main_menu) {
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

            if(ImGui::Button("Quit to Menu and Save")) {
                ui->save_func();
                reset_ui(ui);
            }

            ImGui::End();
        }

        ImGui::Begin("HUD", NULL,
        ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoFocusOnAppearing|
                ImGuiWindowFlags_NoInputs);
        ImGui::SetWindowPos(ImVec2 { 0.0, 0.0 });
        ImGui::SetWindowSize(ImVec2 { (float)prefs->width, (float)prefs->height });

        ImGui::SetWindowFontScale(2.0f);

        // Instructions
        ImGui::Text("Use W A S D to move and SPACE to jump");
        ImGui::Text("Use mouse to look, left click to break, and right click to place.");

        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth()/2 - 8, ImGui::GetWindowHeight()/2 - 8));
        ImGui::Image((void*)*ui->res.atlas, ImVec2(16.0, 16.0), ImVec2(0.0, .5f), ImVec2(1/ATLAS_COLS, 1.0));

        ImGui::End();
    }else {
        ImGui::Begin("Main Menu", NULL,
        ImGuiWindowFlags_NoDecoration);
        ImGui::SetWindowPos(ImVec2 { 0.0, 0.0 });
        ImGui::SetWindowSize(ImVec2 { (float)prefs->width, (float)prefs->height });

        ImGui::SetWindowFontScale(1.5f);

        if(!ui->state.create_menu) {
            if(ImGui::Button("Create New World")) {
                ui->state.create_menu = true;
            }

            if(ImGui::Button("Load Save")) {
                // Consume name
                int item = 0;
                int idx = 0;
                while(item < ui->state.save_combo) {
                    if(ui->save_folders[idx] == '\0') item++;
                    idx++;
                }

                std::string path = "";
                while(ui->save_folders[idx] != '\0') {
                    path.push_back(ui->save_folders[idx]);
                    idx++;
                }
                ui->state.save_folder = path;

                assert(ui->setup_func != nullptr);
                ui->setup_func();
                ui->main_menu = false;
            }
            ImGui::SameLine();
            if(ImGui::Combo("##saveselect", &ui->state.save_combo, ui->save_folders)) {
                printf("Selected world %i\n", ui->state.save_combo);
            }

            if(ImGui::Button("Quit")) {
                ui->quit = true;
            }
        }else {
            if(ImGui::Button("Back")) {
                ui->state.create_menu = false;
            }

            ImGui::InputText("Name", ui->state.new_save_name, sizeof(ui->state.new_save_name), ImGuiInputTextFlags_CallbackEdit, fake_callback);
            ImGui::InputInt("Seed", &ui->state.new_world_seed);

            if(ImGui::Button("Create")) {
                ui->state.save_folder = "worlds/";
                ui->state.save_folder.concat(ui->state.new_save_name);
                printf("%s\n", ui->state.save_folder);

                assert(ui->setup_func != nullptr);
                ui->setup_func();
                ui->main_menu = false;
            }
        }

        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return changed;
}