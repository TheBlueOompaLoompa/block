cmake_minimum_required(VERSION 3.15)

project(Block)

file(COPY shaders DESTINATION res)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lGL")

include_directories(${PROJECT_SOURCE_DIR})

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)
include_directories(${GL_INCLUDE_DIRS})

file(GLOB SOURCES src/*.cpp)
add_executable(mblock ${SOURCES})

target_link_libraries(mblock ${SDL2_LIBRARIES})
target_link_libraries(mblock ${GLEW_LIBRARIES})
target_link_libraries(mblock ${OPENGL_gl_LIBRARY})

target_include_directories(mblock PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_custom_command(
    TARGET mblock
    POST_BUILD
    COMMAND zip
    ARGS -r block.zip mblock res
)