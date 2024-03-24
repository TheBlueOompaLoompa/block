use std::{fs::File, io::Read};

use glium;

pub struct Shader {
    pub program: glium::Program
}

impl Shader {
    // Shader path only includes shader name, file has to end with .v.glsl or .f.glsl
    pub fn new(path: &str, display: &glium_sdl2::SDL2Facade) -> Result<Self, std::io::Error> {
        let mut v_file = File::open(format!("{}.v.glsl", path)).unwrap();
        let mut f_file = File::open(format!("{}.f.glsl", path)).unwrap();

        let mut ver_src = String::new();
        v_file.read_to_string(&mut ver_src)?;

        let mut frag_src = String::new();
        f_file.read_to_string(&mut frag_src)?;

        let program = glium::Program::from_source(display, ver_src.as_str(), frag_src.as_str(), None).unwrap();

        Ok(Shader {
            program
        })
    }
}