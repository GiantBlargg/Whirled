#![feature(cursor_remaining)]
#![feature(try_trait_v2)]

mod assets;
mod ui;
mod wrl;

use bevy::{
	prelude::{App, AssetPlugin, PluginGroup},
	DefaultPlugins,
};

use crate::{assets::LR2AssetPlugin, ui::UIPlugin, wrl::WRLPlugin};

fn main() {
	App::new()
		.add_plugins(
			DefaultPlugins
				.build()
				.add_before::<AssetPlugin, _>(LR2AssetPlugin),
		)
		.add_plugin(WRLPlugin)
		.add_plugin(UIPlugin)
		.run();
}
