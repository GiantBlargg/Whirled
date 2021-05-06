mod container;
mod main;
mod render;

pub use main::whirled;

pub struct MeshDef {
	pub stride: u32,
	pub vector_offset: Option<u32>,
	pub normal_offset: Option<u32>,
	pub colour_offset: Option<u32>,
	pub texcoords_offsets: Vec<u32>,
	pub vertex_buffer: Vec<u8>,
	pub index_buffer: Vec<u16>,
}

pub type ModelDef = Vec<MeshDef>;

pub struct ModelInstance<Render: RenderInterface + ?Sized> {
	pub model: Render::ModelHandle,
}

pub trait RenderInterface {
	type ModelHandle: Copy;
	fn add_model(&mut self, model: ModelDef) -> Self::ModelHandle;

	fn render<'a>(&mut self, models: impl Iterator<Item = ModelInstance<Self>>);
}

pub trait ContentController<TrackedState, Render: RenderInterface> {
	fn new() -> Self;
	fn new_state(&mut self) -> TrackedState;
	fn render(&mut self, state: &TrackedState, render: &mut Render);
}

pub trait Content: 'static {
	type TrackedState;
	type ContentController<Render: RenderInterface>: ContentController<Self::TrackedState, Render>;
}
