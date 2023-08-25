use std::{
	fs, io,
	path::{Path, PathBuf},
};

use bevy::{
	asset::{AssetIo, AssetIoError, Metadata},
	prelude::{AssetServer, Plugin, Resource},
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

impl AssetIo for LR2fs {
	fn load_path<'a>(&'a self, path: &'a Path) -> BoxedFuture<'a, Result<Vec<u8>, AssetIoError>> {
		Box::pin(async move {
			self.read(path).map_err(|e| {
				if e.kind() == io::ErrorKind::NotFound {
					AssetIoError::NotFound(path.to_path_buf())
				} else {
					e.into()
				}
			})
		})
	}

	fn read_directory(
		&self,
		path: &Path,
	) -> Result<Box<dyn Iterator<Item = PathBuf>>, AssetIoError> {
		Ok(Box::new(
			self.read_dir(path)?.map(|entry| entry.unwrap().path()),
		))
	}

	fn get_metadata(&self, path: &Path) -> Result<bevy::asset::Metadata, AssetIoError> {
		self.metadata(path)
			.and_then(Metadata::try_from)
			.map_err(|e| {
				if e.kind() == io::ErrorKind::NotFound {
					AssetIoError::NotFound(path.to_path_buf())
				} else {
					e.into()
				}
			})
	}

	fn watch_path_for_changes(
		&self,
		to_watch: &Path,
		_to_reload: Option<PathBuf>,
	) -> Result<(), AssetIoError> {
		Err(AssetIoError::PathWatchError(to_watch.into()))
	}

	fn watch_for_changes(
		&self,
		_configuration: &bevy::asset::ChangeWatcher,
	) -> Result<(), AssetIoError> {
		Err(AssetIoError::PathWatchError("".into()))
	}
}

pub struct LR2AssetPlugin;

impl Plugin for LR2AssetPlugin {
	fn build(&self, app: &mut bevy::prelude::App) {
		let base = std::env::current_dir().unwrap();

		//TODO: Check for LR2

		let fs = LR2fs { base };

		app.insert_resource(fs.clone());
		app.insert_resource(AssetServer::new(fs));
	}
}
