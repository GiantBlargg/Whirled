mod file;
mod md2;

use std::{env, fs::File, path::PathBuf};

use glam::Mat4;

use self::md2::{load_md2, process_model};
use crate::whirled::{Content, ContentController, ModelInstance, RenderInterface, RenderScene};

pub struct LR2State {}

pub struct LR2Controller<Render: RenderInterface> {
	lr2_path: PathBuf,
	model: Option<Render::ModelHandle>,
}

impl<Render: RenderInterface> ContentController<LR2State, Render> for LR2Controller<Render> {
	fn new() -> Self {
		Self {
			lr2_path: PathBuf::from(env::var("LR2_PATH").unwrap()),
			model: None,
		}
	}

	fn new_state(&mut self) -> LR2State {
		LR2State {}
	}

	fn render(&mut self, _: &LR2State, render: &mut Render) -> RenderScene<Render> {
		let model = {
			let lr2_path = &self.lr2_path;
			*self.model.get_or_insert_with(|| {
				render.add_model(
					File::open(lr2_path.join("game data/ADVENTURERS/OBJECTS/MODELS/ADV_FOYER.MD2"))
						.and_then(load_md2)
						.and_then(process_model)
						.unwrap(),
				)
			})
		};
		RenderScene::<Render> {
			models: vec![ModelInstance::<Render> { model }],
			camera: crate::whirled::CameraView {
				transform: Mat4::IDENTITY,
				projection: Mat4::IDENTITY,
			},
		}
	}
}

pub struct LR2Content {}
impl Content for LR2Content {
	type TrackedState = LR2State;
	type ContentController<Render: RenderInterface> = LR2Controller<Render>;
}
