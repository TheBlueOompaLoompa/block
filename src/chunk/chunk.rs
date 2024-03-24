use crate::chunk::block::{Block, BlockType};

pub const CHUNK_SIZE: usize = 16;

pub struct Chunk {
    pub x: i32,
    pub y: i32,
    pub z: i32,

    pub blocks: [[[Block; CHUNK_SIZE]; CHUNK_SIZE]; CHUNK_SIZE],
}

impl Chunk {
    pub fn new(x: i32, y: i32, z: i32) -> Chunk {
        Chunk {
            x, y, z,
            blocks: [[[Block { x: 0, y: 0, z: 0, block_type: BlockType::AIR }; CHUNK_SIZE]; CHUNK_SIZE]; CHUNK_SIZE],
        }
    }
}