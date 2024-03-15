cmake_minimum_required(VERSION 3.15)

project(Block)

file(COPY res DESTINATION .)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lGL -lSDL2_image -O2")

INCLUDE(FindPkgConfig)

PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)
include_directories(${GL_INCLUDE_DIRS})

find_package(glm REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

find_package ( ReactPhysics3D REQUIRED )
include_directories(${REACT_})

file(GLOB SOURCES src/*.cpp)
add_executable(mblock ${SOURCES})

target_link_libraries(mblock ${SDL2_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_gl_LIBRARY} glm::glm ReactPhysics3D::ReactPhysics3D)

target_include_directories(mblock PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_custom_command(
    TARGET mblock
    POST_BUILD
    COMMAND zip
    ARGS -r block.zip mblock res
)