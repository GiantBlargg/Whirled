use glam::{vec3, Mat4};

#[derive(Clone, Copy)]
pub struct CameraView {
	pub transform: Mat4,
	pub projection: Mat4,
}

pub struct CameraController {
	aspect: f32,
	fov: f32,
}

impl Default for CameraController {
	fn default() -> Self {
		Self {
			aspect: 1.0,
			fov: (90f32).to_radians(),
		}
	}
}

impl CameraController {
	pub fn resize(&mut self, aspect: f32) {
		self.aspect = aspect;
	}
	pub fn camera_view(&self) -> CameraView {
		CameraView {
			transform: Mat4::from_translation(vec3(0.0, 0.0, -1.5)),
			projection: Mat4::perspective_infinite_rh(self.fov, self.aspect, 1.0),
		}
	}
}
