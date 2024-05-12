cmake_minimum_required(VERSION 3.15)

project(Block)

# For LSP 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(COPY res DESTINATION .)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -lGL -lSDL2_image -lSDL2_ttf -ldl -O2 -g")

INCLUDE(FindPkgConfig)

PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
PKG_SEARCH_MODULE(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS} ${SDL2TTF_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)
include_directories(${GL_INCLUDE_DIRS})

find_package(glm REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

file(GLOB IMGUI_SOURCES lib/imgui/*.cpp)
set(IMGUI_BACKEND_SOURCES
    lib/imgui/backends/imgui_impl_sdl2.cpp
    lib/imgui/backends/imgui_impl_opengl3.cpp
)

file(GLOB SOURCES src/*.cpp)
add_executable(mblock ${SOURCES} ${IMGUI_SOURCES} ${IMGUI_BACKEND_SOURCES})

target_link_libraries(mblock ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARY} ${SDL2TTF_LIBRARY} ${GLEW_LIBRARIES} ${OPENGL_gl_LIBRARY} glm::glm)

target_include_directories(mblock PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(mblock PRIVATE lib/imgui)
target_include_directories(mblock PRIVATE ${GL_INCLUDE_DIRS})

add_custom_command(
    TARGET mblock
    POST_BUILD
    COMMAND zip
    ARGS -r block.zip mblock res
)
