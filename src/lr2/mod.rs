mod file;
mod md2;

use std::{env, fs::File, path::Path};

use self::md2::{load_md2, process_model};
use crate::whirled::{
	render::{ModelInstance, RenderInterface},
	Content,
};

pub struct LR2Content {}

impl Content for LR2Content {
	type TrackedState = ();

	fn new() -> Self {
		Self {}
	}

	fn new_state() {}

	fn render<Render: RenderInterface>(&self, _: &Self::TrackedState, render: &mut Render) {
		let _lr2_path = env::var("LR2_PATH").unwrap();
		let lr2_path = Path::new(&_lr2_path);
		let model = render.add_model(
			File::open(lr2_path.join("game data/ADVENTURERS/OBJECTS/MODELS/ADV_FOYER.MD2"))
				.and_then(load_md2)
				.and_then(process_model)
				.unwrap(),
		);
		render.render(std::iter::once(ModelInstance::<Render> { model }));
	}
}
