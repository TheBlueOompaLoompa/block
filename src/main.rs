#[macro_use]
extern crate glium;

extern crate glium_sdl2;
extern crate sdl2;

use ndarray::prelude::*;

mod shader;
use shader::Shader;

mod chunk;
use chunk::Chunk;

mod geometry;
use geometry::Vertex;

fn main() {
    use glium_sdl2::DisplayBuild;
    use glium::Surface;

    let sdl_context = sdl2::init().unwrap();
    let video_subsystem = sdl_context.video().unwrap();

    let display = video_subsystem.window("Block", 1280, 720)
        .resizable()
        .allow_highdpi()
        .build_glium()
        .unwrap();

    implement_vertex!(Vertex, position);

    let chunk = Chunk::new(0, 0, 0);

    let vertex1 = Vertex { position: [-1.0, -1.0, 0.0] };
    let vertex2 = Vertex { position: [ 1.0, -1.0, 0.0] };
    let vertex3 = Vertex { position: [ 0.0,  1.0, 0.0] };
    let shape = vec![vertex1, vertex2, vertex3];

    let mut matrix = Array2::<f32>::zeros((4, 4));
    matrix[[0, 0]] = 1.0;
    matrix[[1, 1]] = 1.0;
    matrix[[2, 2]] = 1.0;
    matrix[[3, 3]] = 1.0;

    uniform! {
        matrix: matrix.
    };

    let vertex_buffer = glium::VertexBuffer::new(&display, &shape).unwrap();
    let indices = glium::index::NoIndices(glium::index::PrimitiveType::TrianglesList);

    let shader = Shader::new("res/shaders/triangle", &display).unwrap();

    let mut running = true;
    let mut event_pump = sdl_context.event_pump().unwrap();

    while running {
        let mut target = display.draw();
        target.clear_color(0.0, 0.0, 1.0, 1.0);
        target.draw(&vertex_buffer, &indices, &shader.program, &glium::uniforms::EmptyUniforms,
                    &glium::DrawParameters{ backface_culling: glium::BackfaceCullingMode::CullClockwise, ..Default::default() }).unwrap();
        target.finish().unwrap();

        for event in event_pump.poll_iter() {
            use sdl2::event::Event;

            match event {
                Event::Quit { .. } => running = false,
                _ => ()
            }
        }
    }
}
