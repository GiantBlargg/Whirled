use std::{
	ffi::CStr,
	io::{self, Read},
	path::{Path, PathBuf},
};

use bevy::prelude::{
	error, BuildChildren, Commands, Component, DespawnRecursiveExt, Entity, Event, EventReader,
	Name, Plugin, Quat, Res, Vec2, Vec3,
};

use crate::assets::LR2fs;

trait ReadExt: io::Read {
	fn read_u32(&mut self) -> io::Result<u32> {
		let mut bytes = [0; 4];
		self.read_exact(&mut bytes)?;
		Ok(u32::from_le_bytes(bytes))
	}

	fn read_f32(&mut self) -> io::Result<f32> {
		let mut bytes = [0; 4];
		self.read_exact(&mut bytes)?;
		Ok(f32::from_le_bytes(bytes))
	}

	fn read_vec2(&mut self) -> io::Result<Vec2> {
		let x = self.read_f32()?;
		let y = self.read_f32()?;
		Ok(Vec2 { x, y })
	}

	fn read_vec3(&mut self) -> io::Result<Vec3> {
		let x = self.read_f32()?;
		let y = self.read_f32()?;
		let z = self.read_f32()?;
		Ok(Vec3 { x, y, z })
	}

	fn read_quat(&mut self) -> io::Result<Quat> {
		let x = self.read_f32()?;
		let y = self.read_f32()?;
		let z = self.read_f32()?;
		let w = self.read_f32()?;
		Ok(Quat::from_xyzw(x, y, z, w))
	}

	fn read_string<const LEN: usize>(&mut self) -> io::Result<String> {
		let mut bytes = [0; LEN];
		self.read_exact(&mut bytes)?;
		CStr::from_bytes_until_nul(&bytes)
			.map_or_else(|_| std::str::from_utf8(&bytes), CStr::to_str)
			.map(str::to_owned)
			.map_err(|err| io::Error::new(io::ErrorKind::InvalidData, err))
	}
}
impl<R: io::Read> ReadExt for R {}

trait WriteExt: io::Write {
	fn write_u32(&mut self, u: u32) -> io::Result<()> {
		self.write_all(&u.to_le_bytes())
	}

	fn write_f32(&mut self, f: f32) -> io::Result<()> {
		self.write_all(&f.to_le_bytes())
	}

	fn write_vec2(&mut self, v: Vec2) -> io::Result<()> {
		self.write_f32(v.x)?;
		self.write_f32(v.y)
	}

	fn write_vec3(&mut self, v: Vec3) -> io::Result<()> {
		self.write_f32(v.x)?;
		self.write_f32(v.y)?;
		self.write_f32(v.z)
	}

	fn write_quat(&mut self, q: Quat) -> io::Result<()> {
		self.write_f32(q.x)?;
		self.write_f32(q.y)?;
		self.write_f32(q.z)?;
		self.write_f32(q.w)
	}

	fn write_string<const LEN: usize>(&mut self, s: &str) -> io::Result<()> {
		if s.len() > LEN {
			return io::Result::Err(io::Error::new(
				io::ErrorKind::InvalidInput,
				"String too long",
			));
		}
		self.write_all(s.as_bytes())?;
		self.write_all(&vec![0u8; LEN - s.len()])
	}
}
impl<W: io::Write> WriteExt for W {}

#[derive(Component)]
pub struct WRLRoot {
	path: PathBuf,
}

#[derive(Component)]
pub struct WRLEntry {
	entry_type: String,
	u: u32,
	layer: u32,
}

#[derive(Component)]
pub struct WRLData {
	data: Vec<u8>,
}

const WRL_MAGIC: u32 = u32::from_le_bytes(*b"RC2W");
const WRL_VERSION: u32 = 11;
const OBMG_MAGIC: u32 = u32::from_le_bytes(*b"OBMG");

fn load_wrl<P: AsRef<Path>>(cmd: &mut Commands, fs: &LR2fs, path: P) -> io::Result<Entity> {
	let mut data = io::Cursor::new(fs.read(path.as_ref())?);

	if data.read_u32()? != WRL_MAGIC {
		return Err(io::Error::new(io::ErrorKind::InvalidData, "Not a WRL file"));
	}
	if data.read_u32()? != WRL_VERSION {
		return Err(io::Error::new(
			io::ErrorKind::InvalidData,
			"Wrong WRL version",
		));
	}

	let root = cmd
		.spawn(WRLRoot {
			path: path.as_ref().to_path_buf(),
		})
		.id();

	while !data.is_empty() {
		if data.read_u32()? != OBMG_MAGIC {
			return Err(io::Error::new(
				io::ErrorKind::InvalidData,
				"Couldn't find OBMG header",
			));
		}

		let entry_type = data.read_string::<24>()?;
		let u = data.read_u32()?;

		let entry_length = data.read_u32()?;
		let next_entry = data.position() + entry_length as u64;

		let layer = data.read_u32()?;
		let name = data.read_string::<24>()?;

		let mut entry_data = vec![0u8; (next_entry - data.position()) as usize];
		data.read_exact(&mut entry_data)?;

		cmd.spawn((
			WRLEntry {
				entry_type,
				u,
				layer,
			},
			Name::new(name),
			WRLData { data: entry_data },
		))
		.set_parent(root);

		if data.position() != next_entry {
			data.set_position(next_entry);
		}
	}

	Ok(root)
}

#[derive(Event)]
pub enum WRLCommand {
	Load(PathBuf),
	Save(Entity),
	Close(Entity),
}

fn wrl_command_system(mut cmd: Commands, mut wrl_cmd: EventReader<WRLCommand>, fs: Res<LR2fs>) {
	for c in wrl_cmd.iter() {
		match c {
			WRLCommand::Load(path) => {
				if let Err(err) = load_wrl(&mut cmd, fs.as_ref(), path) {
					error!(%err);
				};
			}
			WRLCommand::Save(id) => {}
			WRLCommand::Close(id) => {
				cmd.entity(*id).despawn_recursive();
			}
		}
	}
}

pub struct WRLPlugin;

impl Plugin for WRLPlugin {
	fn build(&self, app: &mut bevy::prelude::App) {
		app.add_event::<WRLCommand>().add_system(wrl_command_system);
	}
}
