use std::{
	collections::HashMap,
	env,
	path::Path,
	sync::mpsc::{channel, Receiver, Sender},
	thread,
};

use glam::{Quat, Vec3};
use log::error;

use crate::models::load_model;

type LR2Path = String;

pub struct LR2Model {
	pub path: LR2Path,
	pub pos: Vec3,
	pub quat: Quat,
}

struct Transform {
	pub pos: Vec3,
	pub quat: Quat,
}

struct LR2ModelBatch {
	path: LR2Path,
	trans: Vec<Transform>,
}

enum Asset<T> {
	Pending,
	Complete(T),
}

enum WorkerCommand {
	LoadModel(LR2Path),
}

enum WorkerResponse {
	Model,
}

pub struct LR2Assets {
	models: HashMap<LR2Path, Asset<()>>,
	work_thread: (Sender<WorkerCommand>, Receiver<WorkerResponse>),
}

fn worker(commands: Receiver<WorkerCommand>, response: Sender<WorkerResponse>) {
	while let Ok(command) = commands.recv() {
		match worker_command_handle(command) {
			Ok(_) => {}
			Err(err) => {
				error!("{}", err);
			}
		}
	}
}

fn worker_command_handle(command: WorkerCommand) -> std::io::Result<()> {
	let _lr2_path = env::var("LR2_PATH").unwrap();
	let lr2_path = Path::new(&_lr2_path);
	match command {
		WorkerCommand::LoadModel(path) => {
			load_model(std::fs::File::open(lr2_path.join(path))?)?;
		}
	}
	Ok(())
}

impl LR2Assets {
	pub fn new() -> Self {
		let (host_tx, work_rx) = channel();
		let (work_tx, host_rx) = channel();
		thread::spawn(|| worker(work_rx, work_tx));
		Self {
			models: HashMap::new(),
			work_thread: (host_tx, host_rx),
		}
	}
	pub fn process_models(&mut self, models: impl Iterator<Item = LR2Model>) {
		let model_batches = {
			let mut batches: HashMap<String, LR2ModelBatch> = HashMap::new();
			for m in models {
				batches
					.entry(m.path.clone())
					.or_insert(LR2ModelBatch {
						path: m.path.clone(),
						trans: Vec::new(),
					})
					.trans
					.push(Transform {
						pos: m.pos,
						quat: m.quat,
					});
			}
			batches.into_iter().map(|(_, b)| b)
		};
		let _ = {
			let commands = &self.work_thread.0;
			let models = &mut self.models;

			let _: Vec<()> = model_batches
				.filter_map(|b| {
					if let Asset::Complete(asset) =
						models.entry(b.path.clone()).or_insert_with(|| {
							commands
								.send(WorkerCommand::LoadModel(b.path.clone()))
								.unwrap();
							Asset::Pending
						}) {
						Some(asset.clone())
					} else {
						None
					}
				})
				.collect();
		};
	}
}
