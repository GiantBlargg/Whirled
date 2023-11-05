use std::{
	fs::{self, File},
	io::{self, Read},
	path::{Path, PathBuf},
	pin::Pin,
	task::Poll,
};

use bevy::{
	asset::io::{AssetReader, AssetReaderError, AssetSource, PathStream, Reader},
	prelude::{AssetApp, Plugin, Resource},
	utils::BoxedFuture,
};

#[derive(Resource, Clone)]
pub struct LR2fs {
	base: PathBuf,
}

pub struct MappedDirEntry {
	path: PathBuf,
	dir_entry: fs::DirEntry,
}

impl MappedDirEntry {
	pub fn path(&self) -> PathBuf {
		self.path.clone()
	}
	#[allow(dead_code)]
	pub fn metadata(&self) -> io::Result<fs::Metadata> {
		self.dir_entry.metadata()
	}
	pub fn file_type(&self) -> io::Result<fs::FileType> {
		self.dir_entry.file_type()
	}
	pub fn file_name(&self) -> std::ffi::OsString {
		self.dir_entry.file_name()
	}
}

impl LR2fs {
	pub fn base_name(&self) -> Option<&str> {
		self.base.file_name()?.to_str()
	}

	fn resolve<P: AsRef<Path>>(&self, path: P) -> io::Result<PathBuf> {
		let mut path_buf = self.base.clone();
		for a in path.as_ref().iter() {
			path_buf.push(
				fs::read_dir(&path_buf)?
					.find(|p| {
						p.as_ref()
							.map(|p| p.file_name().to_ascii_lowercase() == a.to_ascii_lowercase())
							.unwrap_or(false)
					})
					.unwrap_or(io::Result::Err(io::Error::from(io::ErrorKind::NotFound)))?
					.file_name(),
			);
		}
		Ok(path_buf)
	}

	pub fn metadata<P: AsRef<Path>>(&self, path: P) -> io::Result<fs::Metadata> {
		fs::metadata(self.resolve(path)?)
	}

	pub fn read<P: AsRef<Path>>(&self, path: P) -> io::Result<Vec<u8>> {
		fs::read(self.resolve(path)?)
	}

	pub fn open<P: AsRef<Path>>(&self, path: P) -> io::Result<File> {
		File::open(self.resolve(path)?)
	}

	pub fn read_dir<P: AsRef<Path>>(
		&self,
		path: P,
	) -> io::Result<impl Iterator<Item = io::Result<MappedDirEntry>> + 'static> {
		let p: PathBuf = path.as_ref().to_owned();
		fs::read_dir(self.resolve(path)?).map(move |r| {
			r.map(move |d| {
				d.map(|d| MappedDirEntry {
					path: p.join(d.file_name()),
					dir_entry: d,
				})
			})
		})
	}
}

struct FileReader(File);

impl futures_io::AsyncRead for FileReader {
	fn poll_read(
		self: Pin<&mut Self>,
		_cx: &mut std::task::Context<'_>,
		buf: &mut [u8],
	) -> Poll<std::io::Result<usize>> {
		let this = self.get_mut();
		let read = this.0.read(buf);
		Poll::Ready(read)
	}
}

struct DirReader(Vec<PathBuf>);

impl futures_core::Stream for DirReader {
	type Item = PathBuf;

	fn poll_next(
		self: Pin<&mut Self>,
		_cx: &mut std::task::Context<'_>,
	) -> Poll<Option<Self::Item>> {
		Poll::Ready(self.get_mut().0.pop())
	}

	fn size_hint(&self) -> (usize, Option<usize>) {
		let len = self.0.len();
		(len, Some(len))
	}
}

impl AssetReader for LR2fs {
	fn read<'a>(
		&'a self,
		path: &'a Path,
	) -> BoxedFuture<'a, Result<Box<Reader<'a>>, AssetReaderError>> {
		Box::pin(async move {
			match self.open(path) {
				Ok(file) => {
					let reader: Box<Reader> = Box::new(FileReader(file));
					Ok(reader)
				}
				Err(e) => {
					if e.kind() == std::io::ErrorKind::NotFound {
						Err(AssetReaderError::NotFound(path.to_owned()))
					} else {
						Err(e.into())
					}
				}
			}
		})
	}

	fn read_meta<'a>(
		&'a self,
		path: &'a Path,
	) -> BoxedFuture<'a, Result<Box<Reader<'a>>, AssetReaderError>> {
		Box::pin(async { Err(AssetReaderError::NotFound(path.to_owned())) })
	}

	fn read_directory<'a>(
		&'a self,
		path: &'a Path,
	) -> BoxedFuture<'a, Result<Box<PathStream>, AssetReaderError>> {
		Box::pin(async move {
			match self.read_dir(path) {
				Ok(read_dir) => {
					let mapped_stream =
						read_dir.filter_map(|f| f.ok().map(|dir_entry| dir_entry.path()));
					let read_dir: Box<PathStream> = Box::new(DirReader(mapped_stream.collect()));
					Ok(read_dir)
				}
				Err(e) => {
					if e.kind() == std::io::ErrorKind::NotFound {
						Err(AssetReaderError::NotFound(path.to_owned()))
					} else {
						Err(e.into())
					}
				}
			}
		})
	}

	fn is_directory<'a>(
		&'a self,
		path: &'a Path,
	) -> BoxedFuture<'a, Result<bool, AssetReaderError>> {
		Box::pin(async move {
			let metadata = self
				.metadata(path)
				.map_err(|_e| AssetReaderError::NotFound(path.to_owned()))?;
			Ok(metadata.file_type().is_dir())
		})
	}
}

pub struct LR2AssetPlugin;

impl Plugin for LR2AssetPlugin {
	fn build(&self, app: &mut bevy::prelude::App) {
		let base = std::env::current_dir().unwrap();

		//TODO: Check for LR2

		let fs = LR2fs { base };

		app.insert_resource(fs.clone());
		app.register_asset_source(
			None,
			AssetSource::build().with_reader(move || Box::new(fs.clone())),
		);
	}
}
