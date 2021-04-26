use std::io::{Error, ErrorKind, Result};

use byteorder::ReadBytesExt;

use super::md2::{load_md2, MD2Chunk, MD2};

pub struct Surface {
	stride: u32,
	vector_offset: Option<u32>,
	normal_offset: Option<u32>,
	colour_offset: Option<u32>,
	texcoords_offsets: Vec<u32>,
	vertex_buffer: Vec<u8>,
	index_buffer: Vec<u16>,
}

pub type Model = Vec<Surface>;

fn process_model(md2: MD2) -> Result<Model> {
	let geo1 = md2
		.iter()
		.find_map(|chunk| {
			if let MD2Chunk::GEO1(geo1) = chunk {
				Some(geo1)
			} else {
				None
			}
		})
		.ok_or(Error::new(
			ErrorKind::InvalidData,
			"File did not contain GEO1",
		))?;

	let surfaces = &geo1
		.first()
		.ok_or(Error::new(
			ErrorKind::InvalidData,
			"GEO1 did not contain any detail levels",
		))?
		.render_groups;

	Ok(surfaces
		.iter()
		.map(|surface| Surface {
			stride: surface.vertex.size_vertstruct,
			vector_offset: {
				if surface.vertex.flags & 0b0001 == 0b0001 {
					Some(surface.vertex.offset_vector)
				} else {
					None
				}
			},
			normal_offset: {
				if surface.vertex.flags & 0b0001 == 0b0001 {
					Some(surface.vertex.offset_normal)
				} else {
					None
				}
			},
			colour_offset: {
				if surface.vertex.flags & 0b0001 == 0b0001 {
					Some(surface.vertex.offset_colour)
				} else {
					None
				}
			},
			texcoords_offsets: {
				if surface.vertex.flags & 0b0001 == 0b0001 {
					vec![surface.vertex.offset_texcoord]
				} else {
					Vec::new()
				}
			},
			vertex_buffer: surface.vertex.buffer.clone(),
			index_buffer: surface.fill.indicies.clone(),
		})
		.collect())
}

pub fn load_model(file: impl ReadBytesExt) -> Result<Model> {
	process_model(load_md2(file)?)
}
