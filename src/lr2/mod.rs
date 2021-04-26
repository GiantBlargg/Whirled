mod file;
mod md2;

use std::{env, fs::File, path::Path};

use gfx_hal::Backend;

use self::md2::load_md2;
use crate::whirled::{render::Render, Content};

pub struct LR2Content {}

impl<B: Backend> Content<B> for LR2Content {
	type TrackedState = ();

	fn new() -> Self {
		Self {}
	}

	fn new_state() {}

	fn render(&self, _: &Self::TrackedState, render: &mut Render<B>) {
		let _lr2_path = env::var("LR2_PATH").unwrap();
		let lr2_path = Path::new(&_lr2_path);
		let model = File::open(lr2_path.join("game data/ADVENTURERS/OBJECTS/MODELS/ADV_FOYER.MD2"))
			.and_then(load_md2)
			.unwrap();
		render.render();
	}
}
