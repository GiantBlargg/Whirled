use assets::LR2AssetPlugin;
use bevy::{
	prelude::{App, AssetPlugin, PluginGroup},
	DefaultPlugins,
};
use ui::UIPlugin;
mod assets;
mod ui;

fn main() {
	App::new()
		.add_plugins(
			DefaultPlugins
				.build()
				.add_before::<AssetPlugin, _>(LR2AssetPlugin),
		)
		.add_plugin(UIPlugin)
		.run();
}
