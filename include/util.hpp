#pragma once
#ifndef UTIL_H

#define UTIL_H 1

#include <SDL2/SDL.h>
#include <SDL2/SDL_rwops.h>
#include <sys/stat.h>
#include <filesystem>

// COPY: From a tutorial on how to load shader files
char* file_read(const char* filename);

void chk_mkdir(const char* dir_path);
void save_data(const char* fname, void* src, size_t size);
void load_data(const char* fname, void* dest, size_t size);
#endif