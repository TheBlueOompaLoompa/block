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
      SDL2_ttf
      libtiff
      freetype
      wayland
      libGL
      libxkbcommon
      glslang
      zip
      gdb
      harfbuzz
      libwebp
      glib
      pcre2
    ];
}
