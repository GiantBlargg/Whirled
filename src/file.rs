use std::io::{Read, Result};

use byteorder::{ByteOrder, ReadBytesExt};
use glam::{vec3, vec4, Vec3, Vec4};
use log::error;

pub trait ReadString: Read {
	fn read_sized_string<const N: usize>(&mut self) -> Result<String> {
		let mut buf = [0; N];
		self.read_exact(&mut buf)?;
		let mut string = "".to_string();
		for char in &buf {
			if *char == 0 {
				break;
			}
			string.push(*char as char);
		}
		Ok(string)
	}
}
impl<T: Read> ReadString for T {}

pub trait ReadVec: ReadBytesExt {
	fn read_vec3<T: ByteOrder>(&mut self) -> Result<Vec3> {
		let x = self.read_f32::<T>()?;
		let y = self.read_f32::<T>()?;
		let z = self.read_f32::<T>()?;
		Ok(vec3(x, y, z))
	}
	fn read_vec4<T: ByteOrder>(&mut self) -> Result<Vec4> {
		let x = self.read_f32::<T>()?;
		let y = self.read_f32::<T>()?;
		let z = self.read_f32::<T>()?;
		let w = self.read_f32::<T>()?;
		Ok(vec4(x, y, z, w))
	}
}
impl<T: ReadBytesExt> ReadVec for T {}

pub trait ReadFatBool: ReadBytesExt {
	fn read_fat_bool<T: ByteOrder>(&mut self) -> Result<bool> {
		let b = self.read_u32::<T>()?;
		match b {
			0 => Ok(false),
			1 => Ok(true),
			_ => {
				error!("Only 0 or 1!");
				Ok(true)
			}
		}
	}
}
impl<T: ReadBytesExt> ReadFatBool for T {}
