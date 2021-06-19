mod camera;
mod container;
mod main;
mod wgpu;

use self::camera::CameraView;
pub use self::{main::whirled, wgpu::Render as WgpuRender};

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

pub struct RenderScene<Render: RenderInterface + ?Sized> {
	pub models: Vec<ModelInstance<Render>>,
}

pub trait RenderInterface {
	type ModelHandle: Copy;
	fn add_model(&mut self, model: ModelDef) -> Self::ModelHandle;
}

pub trait WhirledRender: 'static {
	fn new(window: &impl raw_window_handle::HasRawWindowHandle) -> Self;
	fn resize(&mut self, width: u32, height: u32);

	type RenderInterface: RenderInterface;

	fn get_interface(&mut self) -> &mut Self::RenderInterface;

	fn render(&mut self, camera: CameraView, scene: RenderScene<Self::RenderInterface>);
}

pub trait ContentController<TrackedState, Render: RenderInterface> {
	fn new() -> Self;
	fn new_state(&mut self) -> TrackedState;
	fn render(&mut self, state: &TrackedState, render: &mut Render) -> RenderScene<Render>;
}

pub trait Content: 'static {
	type TrackedState;
	type ContentController<Render: RenderInterface>: ContentController<Self::TrackedState, Render>;
}
