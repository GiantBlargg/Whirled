#![feature(async_closure)]
#![feature(generic_associated_types)]

mod lr2;
mod whirled;

fn main() {
	env_logger::Builder::from_env("WHIRLED_LOG")
		.filter(None, log::LevelFilter::Info)
		.init();

	whirled::whirled::<whirled::WgpuRender, lr2::LR2Content>()
}
