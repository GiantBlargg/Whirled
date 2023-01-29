use std::path::{Path, PathBuf};

use bevy::prelude::{Plugin, Res, ResMut, Resource};
use bevy_egui::{
	egui::{CollapsingHeader, ScrollArea, SidePanel, TopBottomPanel, Ui},
	EguiContext, EguiPlugin,
};

use crate::assets::LR2fs;

#[derive(Resource)]
struct FileRes {
	current_path: PathBuf,
}

fn populate_dir<P: AsRef<Path>>(fs: &LR2fs, fr: &mut ResMut<FileRes>, ui: &mut Ui, path: P) {
	let p = path.as_ref();
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
		let subdir = p.join(&d);
		let selected = subdir == fr.current_path;
		#[allow(deprecated)]
		let resp = CollapsingHeader::new(d)
			.selectable(true)
			.selected(selected)
			.show(ui, |ui| populate_dir(fs, fr, ui, &subdir));
		if resp.header_response.clicked() {
			fr.as_mut().current_path = subdir;
		}
	}
}

fn file_ui(mut ctx: ResMut<EguiContext>, fs: Res<LR2fs>, mut fr: ResMut<FileRes>) {
	TopBottomPanel::bottom("file_ui")
		.resizable(true)
		.show(ctx.ctx_mut(), |ui| {
			SidePanel::left("dir_tree").show_inside(ui, |ui| {
				ScrollArea::vertical().show(ui, |ui| populate_dir(fs.as_ref(), &mut fr, ui, ""));
			});
			ScrollArea::vertical().show(ui, |ui| {
				ui.horizontal_wrapped(|ui| {
					let mut contents: Vec<String> =
						fs.read_dir(&fr.current_path).map_or(vec![], |d| {
							d.filter_map(|e| {
								e.ok().map(|e| e.file_name().to_string_lossy().into_owned())
							})
							.collect()
						});

					contents.sort_unstable_by_key(|d| d.to_ascii_lowercase());

					for c in contents {
						if ui.button(&c).clicked() {
							let p = fr.current_path.join(c);
							if p.extension() == Some(std::ffi::OsStr::new("wrl")) {}
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
			})
			.add_system(file_ui);
	}
}
