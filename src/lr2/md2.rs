use std::{
	io::{Error, ErrorKind, Read, Result},
	u32,
};

use byteorder::{ReadBytesExt, LE};
use glam::{Vec3, Vec4};
use log::error;

use super::file::{ReadFatBool, ReadString, ReadVec};
use crate::whirled::render::{MeshDef, ModelDef};

pub struct BoundingBox {
	pub min: Vec3,
	pub max: Vec3,
	pub center: Vec3,
	pub rot_y: f32,
}
pub struct Bitmap {
	pub path: String,
	pub bitmap_type: u32,
	pub index: u32,
}
pub struct MatProp {
	pub ambient: Vec4,
	pub diffuse: Vec4,
	pub specular: Vec4,
	pub emissive: Vec4,
	pub shine: f32,
	pub alpha: f32,
	pub alphatype: u32,
	pub bitfield: u32,
	pub animname: String,
}
pub struct MDL2 {
	pub inertia_multi: Vec3,
	pub bounding_radius: f32,
	pub distance_fades: u32,
	pub bounding_box: Option<BoundingBox>,
	pub use_unique_materials: u32,
	pub use_unique_textures: u32,
	pub use_generic_geometry: u32,
	pub vertex_buffer_flags: u32,
	pub u0: [u8; 48],
	pub bitmaps: Vec<Bitmap>,
	pub mat_props: Vec<MatProp>,
}

pub struct Blend {
	pub effect: u32,
	pub texture_index: u16,
	pub coordinate_index: u8,
	pub tiling_info: u8,
}
pub struct TexBlend {
	pub effectmask: u16,
	pub render_reference: u16,
	pub effects: u16,
	pub custom: u8,
	pub coordinates: u8,
	pub blends: [Blend; 4],
}
pub struct Vertex {
	pub offset_vector: u32,
	pub offset_normal: u32,
	pub offset_colour: u32,
	pub offset_texcoord: u32,
	pub size_vertstruct: u32,
	pub num_texcoords: u32,
	pub flags: u16,
	pub vertices: u16,
	pub managedbuffer: u16,
	pub currentvertex: u16,
	pub u0: [u8; 8],
	pub buffer: Vec<u8>,
}
pub struct Fill {
	pub selectable_prim_blocks: u32,
	pub fill_type: u32,
	pub indicies: Vec<u16>,
}
pub struct RenderGroup {
	pub polygons: u16,
	pub vertices: u16,
	pub material: u16,
	pub effects: u16,
	pub u0: [u8; 12],
	pub tex_blend: TexBlend,
	pub vertex: Vertex,
	pub fill: Fill,
}
pub struct DetailLevel {
	pub detail_type: u32,
	pub maxed_ge_length: f32,
	pub u0: [u8; 8],
	pub render_groups: Vec<RenderGroup>,
}
pub type GEO1 = Vec<DetailLevel>;

pub enum MD2Chunk {
	MDL0(Vec<u8>),
	MDL1(Vec<u8>),
	MDL2(MDL2),
	GEO0(Vec<u8>),
	GEO1(GEO1),
	P2G0(Vec<u8>),
	COLD(Vec<u8>),
	SHA0(Vec<u8>),
	Unknown(String, Vec<u8>),
}

pub type MD2 = Vec<MD2Chunk>;

pub fn load_md2(mut file: impl Read) -> Result<MD2> {
	let mut md2_file: MD2 = Vec::new();
	loop {
		let chunk_type = file.read_sized_string::<4>()?;
		if chunk_type == "" {
			return Ok(md2_file);
		}

		let chunk_size = file.read_u32::<LE>()? as u64;
		let mut chunk = (&mut file).take(chunk_size);

		md2_file.push(match chunk_type.as_str() {
			"MDL0" => {
				let mut buf = Vec::new();
				chunk.read_to_end(&mut buf)?;
				MD2Chunk::MDL0(buf)
			}
			"MDL1" => {
				let mut buf = Vec::new();
				chunk.read_to_end(&mut buf)?;
				MD2Chunk::MDL1(buf)
			}
			"MDL2" => MD2Chunk::MDL2(MDL2 {
				inertia_multi: chunk.read_vec3::<LE>()?,
				bounding_radius: chunk.read_f32::<LE>()?,
				distance_fades: chunk.read_u32::<LE>()?,
				bounding_box: {
					if chunk.read_fat_bool::<LE>()? {
						Some(BoundingBox {
							min: chunk.read_vec3::<LE>()?,
							max: chunk.read_vec3::<LE>()?,
							center: chunk.read_vec3::<LE>()?,
							rot_y: chunk.read_f32::<LE>()?,
						})
					} else {
						None
					}
				},
				use_unique_materials: chunk.read_u32::<LE>()?,
				use_unique_textures: chunk.read_u32::<LE>()?,
				use_generic_geometry: chunk.read_u32::<LE>()?,
				vertex_buffer_flags: chunk.read_u32::<LE>()?,
				u0: {
					let mut u0 = [0u8; 48];
					chunk.read_exact(&mut u0)?;
					u0
				},
				bitmaps: {
					let mut bitmaps = Vec::with_capacity(chunk.read_u32::<LE>()? as usize);
					for _ in 0..bitmaps.capacity() {
						bitmaps.push(Bitmap {
							path: chunk.read_sized_string::<256>()?,
							bitmap_type: chunk.read_u32::<LE>()?,
							index: chunk.read_u32::<LE>()?,
						})
					}
					bitmaps
				},
				mat_props: {
					let mut mat_props = Vec::with_capacity(chunk.read_u32::<LE>()? as usize);
					for _ in 0..mat_props.capacity() {
						mat_props.push(MatProp {
							ambient: chunk.read_vec4::<LE>()?,
							diffuse: chunk.read_vec4::<LE>()?,
							specular: chunk.read_vec4::<LE>()?,
							emissive: chunk.read_vec4::<LE>()?,
							shine: chunk.read_f32::<LE>()?,
							alpha: chunk.read_f32::<LE>()?,
							alphatype: chunk.read_u32::<LE>()?,
							bitfield: chunk.read_u32::<LE>()?,
							animname: chunk.read_sized_string::<8>()?,
						})
					}
					mat_props
				},
			}),
			"GEO0" => {
				let mut buf = Vec::new();
				chunk.read_to_end(&mut buf)?;
				MD2Chunk::GEO0(buf)
			}
			"GEO1" => MD2Chunk::GEO1({
				let mut detail_levels = Vec::with_capacity(chunk.read_u32::<LE>()? as usize);
				for _ in 0..detail_levels.capacity() {
					detail_levels.push({
						let detail_type = chunk.read_u32::<LE>()?;
						let maxed_ge_length = chunk.read_f32::<LE>()?;
						let render_groups_count = chunk.read_u32::<LE>()?;
						let u0 = {
							let mut u0 = [0u8; 8];
							chunk.read_exact(&mut u0)?;
							u0
						};
						DetailLevel {
							detail_type,
							maxed_ge_length,
							u0,
							render_groups: {
								let mut render_groups =
									Vec::with_capacity(render_groups_count as usize);
								for _ in 0..render_groups.capacity() {
									render_groups.push(RenderGroup {
										polygons: chunk.read_u16::<LE>()?,
										vertices: chunk.read_u16::<LE>()?,
										material: chunk.read_u16::<LE>()?,
										effects: chunk.read_u16::<LE>()?,
										u0: {
											let mut u0 = [0u8; 12];
											chunk.read_exact(&mut u0)?;
											u0
										},
										tex_blend: TexBlend {
											effectmask: chunk.read_u16::<LE>()?,
											render_reference: chunk.read_u16::<LE>()?,
											effects: chunk.read_u16::<LE>()?,
											custom: chunk.read_u8()?,
											coordinates: chunk.read_u8()?,
											blends: {
												[
													Blend {
														effect: chunk.read_u32::<LE>()?,
														texture_index: chunk.read_u16::<LE>()?,
														coordinate_index: chunk.read_u8()?,
														tiling_info: chunk.read_u8()?,
													},
													Blend {
														effect: chunk.read_u32::<LE>()?,
														texture_index: chunk.read_u16::<LE>()?,
														coordinate_index: chunk.read_u8()?,
														tiling_info: chunk.read_u8()?,
													},
													Blend {
														effect: chunk.read_u32::<LE>()?,
														texture_index: chunk.read_u16::<LE>()?,
														coordinate_index: chunk.read_u8()?,
														tiling_info: chunk.read_u8()?,
													},
													Blend {
														effect: chunk.read_u32::<LE>()?,
														texture_index: chunk.read_u16::<LE>()?,
														coordinate_index: chunk.read_u8()?,
														tiling_info: chunk.read_u8()?,
													},
												]
											},
										},
										vertex: {
											let offset_vector = chunk.read_u32::<LE>()?;
											let offset_normal = chunk.read_u32::<LE>()?;
											let offset_colour = chunk.read_u32::<LE>()?;
											let offset_texcoord = chunk.read_u32::<LE>()?;
											let size_vertstruct = chunk.read_u32::<LE>()?;
											let num_texcoords = chunk.read_u32::<LE>()?;
											let flags = chunk.read_u16::<LE>()?;
											let vertices = chunk.read_u16::<LE>()?;
											let managedbuffer = chunk.read_u16::<LE>()?;
											let currentvertex = chunk.read_u16::<LE>()?;
											let u0 = {
												let mut u0 = [0u8; 8];
												chunk.read_exact(&mut u0)?;
												u0
											};
											let mut buffer = Vec::with_capacity(
												vertices as usize * size_vertstruct as usize,
											);

											(&mut chunk)
												.take(vertices as u64 * size_vertstruct as u64)
												.read_to_end(&mut buffer)?;
											Vertex {
												offset_vector,
												offset_normal,
												offset_colour,
												offset_texcoord,
												size_vertstruct,
												num_texcoords,
												flags,
												vertices,
												managedbuffer,
												currentvertex,
												u0,
												buffer,
											}
										},
										fill: Fill {
											selectable_prim_blocks: chunk.read_u32::<LE>()?,
											fill_type: chunk.read_u32::<LE>()?,
											indicies: {
												let mut indicies = Vec::with_capacity(
													chunk.read_u32::<LE>()? as usize,
												);
												for _ in 0..indicies.capacity() {
													indicies.push(chunk.read_u16::<LE>()?);
												}
												indicies
											},
										},
									})
								}
								render_groups
							},
						}
					})
				}
				detail_levels
			}),
			"P2G0" => {
				let mut buf = Vec::new();
				chunk.read_to_end(&mut buf)?;
				MD2Chunk::P2G0(buf)
			}
			"COLD" => {
				let mut buf = Vec::new();
				chunk.read_to_end(&mut buf)?;
				MD2Chunk::COLD(buf)
			}
			"SHA0" => {
				let mut buf = Vec::new();
				chunk.read_to_end(&mut buf)?;
				MD2Chunk::SHA0(buf)
			}
			_ => {
				error!("Unknown chunk {}", chunk_type);
				let mut buf = Vec::new();
				chunk.read_to_end(&mut buf)?;
				MD2Chunk::Unknown(chunk_type, buf)
			}
		});
		{
			let mut buf = Vec::new();
			chunk.read_to_end(&mut buf)?;
			if buf.len() != 0 {
				error!("Extra Data in Chunk");
			}
		}
	}
}

pub fn process_model(md2: MD2) -> Result<ModelDef> {
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
		.map(|surface| MeshDef {
			stride: surface.vertex.size_vertstruct,
			vector_offset: {
				if surface.vertex.flags & 0b0001 == 0b0001 {
					Some(surface.vertex.offset_vector)
				} else {
					None
				}
			},
			normal_offset: {
				if surface.vertex.flags & 0b0010 == 0b0010 {
					Some(surface.vertex.offset_normal)
				} else {
					None
				}
			},
			colour_offset: {
				if surface.vertex.flags & 0b0100 == 0b0100 {
					Some(surface.vertex.offset_colour)
				} else {
					None
				}
			},
			texcoords_offsets: {
				if surface.vertex.flags & 0b1000 == 0b1000 {
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
