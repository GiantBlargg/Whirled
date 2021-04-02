use std::io::Result;

use byteorder::ReadBytesExt;

mod md2 {
	use std::{
		io::{Read, Result},
		u32,
	};

	use byteorder::{ReadBytesExt, LE};
	use glam::{Vec3, Vec4};
	use log::error;

	use crate::file::{ReadFatBool, ReadString, ReadVec};

	struct BoundingBox {
		min: Vec3,
		max: Vec3,
		center: Vec3,
		rot_y: f32,
	}
	struct Bitmap {
		path: String,
		bitmap_type: u32,
		index: u32,
	}
	struct MatProp {
		ambient: Vec4,
		diffuse: Vec4,
		specular: Vec4,
		emissive: Vec4,
		shine: f32,
		alpha: f32,
		alphatype: u32,
		bitfield: u32,
		animname: String,
	}
	pub struct MDL2 {
		inertia_multi: Vec3,
		bounding_radius: f32,
		distance_fades: u32,
		bounding_box: Option<BoundingBox>,
		use_unique_materials: u32,
		use_unique_textures: u32,
		use_generic_geometry: u32,
		vertex_buffer_flags: u32,
		u0: [u8; 48],
		bitmaps: Vec<Bitmap>,
		mat_props: Vec<MatProp>,
	}

	struct Blend {
		effect: u32,
		texture_index: u16,
		coordinate_index: u8,
		tiling_info: u8,
	}
	struct TexBlend {
		effectmask: u16,
		render_reference: u16,
		effects: u16,
		custom: u8,
		coordinates: u8,
		blends: [Blend; 4],
	}
	struct Vertex {
		offset_vector: u32,
		offset_normal: u32,
		offset_colour: u32,
		offset_texcoord: u32,
		size_vertstruct: u32,
		num_texcoords: u32,
		flags: u16,
		vertices: u16,
		managedbuffer: u16,
		currentvertex: u16,
		u0: [u8; 8],
		buffer: Vec<u8>,
	}
	struct Fill {
		selectable_prim_blocks: u32,
		fill_type: u32,
		indicies: Vec<u16>,
	}
	struct RenderGroup {
		polygons: u16,
		vertices: u16,
		material: u16,
		effects: u16,
		u0: [u8; 12],
		tex_blend: TexBlend,
		vertex: Vertex,
		fill: Fill,
	}
	pub struct DetailLevel {
		detail_type: u32,
		maxed_ge_length: f32,
		u0: [u8; 8],
		render_groups: Vec<RenderGroup>,
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

	pub fn load_md2(mut file: impl ReadBytesExt) -> Result<MD2> {
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
															texture_index: chunk
																.read_u16::<LE>()?,
															coordinate_index: chunk.read_u8()?,
															tiling_info: chunk.read_u8()?,
														},
														Blend {
															effect: chunk.read_u32::<LE>()?,
															texture_index: chunk
																.read_u16::<LE>()?,
															coordinate_index: chunk.read_u8()?,
															tiling_info: chunk.read_u8()?,
														},
														Blend {
															effect: chunk.read_u32::<LE>()?,
															texture_index: chunk
																.read_u16::<LE>()?,
															coordinate_index: chunk.read_u8()?,
															tiling_info: chunk.read_u8()?,
														},
														Blend {
															effect: chunk.read_u32::<LE>()?,
															texture_index: chunk
																.read_u16::<LE>()?,
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
}

pub fn load_model(file: impl ReadBytesExt) -> Result<()> {
	md2::load_md2(file)?;
	Ok(())
}
