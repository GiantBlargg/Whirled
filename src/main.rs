use assets::LR2AssetPlugin;
use bevy::{
	prelude::{App, AssetPlugin, PluginGroup},
	DefaultPlugins,
};
mod assets;

fn main() {
	App::new()
		.add_plugins(
			DefaultPlugins
				.build()
				.add_before::<AssetPlugin, _>(LR2AssetPlugin),
		)
		.run();
}
