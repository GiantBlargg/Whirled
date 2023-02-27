use std::{
	ffi::OsStr,
	path::{Path, PathBuf},
};

use bevy::prelude::{Plugin, Res, ResMut, Resource};
use bevy_egui::{
	egui::{CollapsingHeader, ScrollArea, SidePanel, TopBottomPanel, Ui},
	EguiContext, EguiPlugin,
};

use crate::assets::LR2fs;

#[derive(Resource)]
struct FileRes {
	current_path: PathBuf,
	queue_open: bool,
}

fn populate_dir<P: AsRef<Path>>(fs: &LR2fs, fr: &mut ResMut<FileRes>, ui: &mut Ui, path: P) {
	let p = path.as_ref();

	let selected = p == fr.current_path;
	let open = fr.queue_open && fr.current_path.starts_with(p);
	let name = (if p == Path::new("") {
		fs.base_name()
	} else {
		p.file_name().and_then(OsStr::to_str)
	})
	.unwrap_or_default();
	#[allow(deprecated)]
	let resp = CollapsingHeader::new(name)
		.selectable(true)
		.selected(selected)
		.open(if open { Some(true) } else { None })
		.show(ui, |ui| {
			let mut dirs: Vec<_> = fs.read_dir(p).map_or(vec![], |d| {
				d.filter_map(|e| {
					e.ok().and_then(|e| {
						if e.file_type().map_or(false, |t| t.is_dir()) {
							Some(e.file_name().to_string_lossy().into_owned())
						} else {
							None
						}
					})
				})
				.collect()
			});

			dirs.sort_unstable_by_key(|d| d.to_ascii_lowercase());

			for d in dirs {
				populate_dir(fs, fr, ui, p.join(&d))
			}
		});
	if resp.header_response.clicked() {
		fr.as_mut().current_path = p.to_path_buf();
	}
}

#[derive(PartialEq, Eq)]
enum FileType {
	Unknown,
	Directory,
	Wrl,
	Model,
}

fn file_ui(mut ctx: ResMut<EguiContext>, fs: Res<LR2fs>, mut fr: ResMut<FileRes>) {
	TopBottomPanel::bottom("file_ui")
		.resizable(true)
		.show(ctx.ctx_mut(), |ui| {
			SidePanel::left("dir_tree").show_inside(ui, |ui| {
				ScrollArea::vertical().show(ui, |ui| populate_dir(fs.as_ref(), &mut fr, ui, ""));
			});
			fr.queue_open = false;
			ScrollArea::vertical().show(ui, |ui| {
				ui.horizontal_wrapped(|ui| {
					let mut contents = fs.read_dir(&fr.current_path).map_or(vec![], |d| {
						d.filter_map(|e| {
							e.ok().map(|e| {
								(
									e.file_name().to_string_lossy().into_owned(),
									e.path(),
									match e.file_type() {
										Err(_) => FileType::Unknown,
										Ok(t) => {
											if t.is_dir() {
												FileType::Directory
											} else {
												let ext = e
													.path()
													.extension()
													.and_then(OsStr::to_str)
													.map(str::to_ascii_lowercase)
													.unwrap_or_default();
												match ext.as_str() {
													"wrl" => FileType::Wrl,
													"md2" => FileType::Model,
													_ => FileType::Unknown,
												}
											}
										}
									},
								)
							})
						})
						.collect()
					});

					if let Some(p) = fr.current_path.parent() {
						contents.push(("..".to_owned(), p.to_owned(), FileType::Directory));
					}

					contents.sort_unstable_by_key(|(d, _, t)| {
						(t != &FileType::Directory, d.to_ascii_lowercase())
					});

					for (c, p, t) in contents {
						let resp = ui.button(&c);
						if resp.double_clicked() {
							match t {
								FileType::Unknown => {}
								FileType::Directory => {
									fr.current_path = p;
									fr.queue_open = true;
								}
								FileType::Wrl => {}
								FileType::Model => {}
							}
						}
					}
				})
			});
		});
}

pub struct UIPlugin;

impl Plugin for UIPlugin {
	fn build(&self, app: &mut bevy::prelude::App) {
		app.add_plugin(EguiPlugin)
			.insert_resource(FileRes {
				current_path: "game data/SAVED WORLDS".into(),
				queue_open: true,
			})
			.add_system(file_ui);
	}
}
