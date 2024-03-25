use crate::chunk::block::{Block, BlockType};
use crate::geometry::Vertex;

pub const CHUNK_SIZE: usize = 16;

pub struct Chunk<'a> {
    pub x: i32,
    pub y: i32,
    pub z: i32,

    pub blocks: [[[Block; CHUNK_SIZE]; CHUNK_SIZE]; CHUNK_SIZE],

    pub adjacent_chunks: [Option<&'a Chunk<'a>>; 6],

    pub mesh: Vec<Vertex>
}

impl Chunk<'_> {
    pub fn new(x: i32, y: i32, z: i32) -> Chunk<'static> {
        Chunk {
            x, y, z,
            adjacent_chunks: [None; 6],
            mesh: Vec::new(),
            blocks: [[[Block { x: 0, y: 0, z: 0, block_type: BlockType::AIR }; CHUNK_SIZE]; CHUNK_SIZE]; CHUNK_SIZE],
        }
    }

    pub fn update_mesh() {
        for my in 0..CHUNK_SIZE {
            for mz in 0..CHUNK_SIZE {
                for mx in 0..CHUNK_SIZE {
                    match self.blocks[mx][my][mz].block_type {
                        BlockType::AIR => continue,
                        _ => ()
                    }

                    generate_side(mx, my + 1, mz, V3FORWARD, V3RIGHT,true        );  // TOP
                }
            }
        }
    }

    fn generate_side() {
        
    }
}