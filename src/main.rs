#![feature(generic_associated_types)]

mod lr2;
mod whirled;

fn main() {
	env_logger::Builder::from_env("WHIRLED_LOG")
		.filter(None, log::LevelFilter::Error)
		.init();

	// let mut lr2_assets = LR2Assets::new();

	whirled::whirled::<gfx_backend_vulkan::Backend, lr2::LR2Content>()
}
