#[derive(Copy, Clone)]
pub struct Block {
    pub x: i32,
    pub y: i32,
    pub z: i32,

    pub block_type: BlockType,
}

#[derive(Copy, Clone)]
pub enum BlockType {
    AIR,
    DIRT
}