use std::f32::consts::FRAC_PI_2;

use glam::{vec2, Mat4, Vec2, Vec3};
use log::info;
use winit::{
	event::{ElementState, MouseButton, VirtualKeyCode},
	window::Window,
};

#[derive(Clone, Copy)]
pub struct CameraView {
	pub transform: Mat4,
	pub projection: Mat4,
}

pub struct CameraController {
	fov: f32,
	mouse_speed: Vec2,

	projection: Mat4,
	right: bool,

	pos: Vec3,
	rot: Vec2,
	movement: Vec3,
}

impl Default for CameraController {
	fn default() -> Self {
		Self {
			fov: (90f32).to_radians(),
			mouse_speed: Vec2::ONE * ((0.2f32).to_radians()),
			projection: Mat4::IDENTITY,
			right: false,
			pos: Vec3::ZERO,
			rot: Vec2::ZERO,
			movement: Vec3::ZERO,
		}
	}
}

impl CameraController {
	pub fn resize(&mut self, aspect: f32) {
		self.projection = Mat4::perspective_infinite_rh(self.fov, aspect, 0.1);
	}
	pub fn mouse_button(
		&mut self,
		state: ElementState,
		button: MouseButton,
		window: &Window,
	) -> bool {
		if button == MouseButton::Right {
			window
				.set_cursor_grab(state == ElementState::Pressed)
				.unwrap();
			window.set_cursor_visible(state == ElementState::Released);
			self.right = state == ElementState::Pressed;
			true
		} else {
			false
		}
	}
	pub fn mouse_move(&mut self, x: f32, y: f32) -> bool {
		if self.right {
			self.rot.x += x * self.mouse_speed.x;
			self.rot.y += y * self.mouse_speed.y;
			self.rot.y = self.rot.y.clamp(-FRAC_PI_2, FRAC_PI_2);
			true
		} else {
			false
		}
	}
	pub fn keyboard_input(&mut self, keyboard_input: winit::event::KeyboardInput) -> bool {
		let dir = match keyboard_input.virtual_keycode {
			Some(VirtualKeyCode::W) => -Vec3::Z,
			Some(VirtualKeyCode::S) => Vec3::Z,
			Some(VirtualKeyCode::A) => -Vec3::X,
			Some(VirtualKeyCode::D) => Vec3::X,
			Some(VirtualKeyCode::Q) => -Vec3::Y,
			Some(VirtualKeyCode::E) => Vec3::Y,
			_ => return false,
		};

		let diff = match keyboard_input.state {
			ElementState::Pressed => dir,
			ElementState::Released => -dir,
		};

		self.movement = self.movement + diff;

		true
	}
	pub fn camera_view(&mut self) -> CameraView {
		let rot = Mat4::from_rotation_y(-self.rot.x) * Mat4::from_rotation_x(-self.rot.y);

		if self.right {
			self.pos = self.pos + rot.transform_point3(self.movement / 60.0);
		}

		CameraView {
			transform: (Mat4::from_translation(self.pos) * rot).inverse(),
			projection: self.projection,
		}
	}
}
