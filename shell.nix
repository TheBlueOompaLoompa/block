{ pkgs ? import <nixpkgs> {} }:
  pkgs.mkShell {
    # nativeBuildInputs is usually what you want -- tools you need to run
    nativeBuildInputs = with pkgs.buildPackages; [
      cmake
      pkg-config
      glew
      freeglut
      glfw
      glm
      mesa
      SDL2
      SDL2_image
      freetype
      glslang
      zip
      gdb
    ];
}
