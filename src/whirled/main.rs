use std::time::Instant;

use winit::{
	event::{DeviceEvent, Event, WindowEvent},
	event_loop::{ControlFlow, EventLoop},
	window::WindowBuilder,
};

use super::{camera::CameraController, Content, ContentController, WhirledRender};

pub fn whirled<Render: WhirledRender, C: Content>() {
	let event_loop = EventLoop::new();

	let window = WindowBuilder::new()
		.with_title("Whirled")
		.with_visible(false)
		.build(&event_loop)
		.unwrap();

	let mut imgui = imgui::Context::create();

	let mut render = Render::new(&window);

	let mut platform = imgui_winit_support::WinitPlatform::init(&mut imgui);
	platform.attach_window(
		imgui.io_mut(),
		&window,
		imgui_winit_support::HiDpiMode::Default,
	);

	let mut camera = CameraController::default();

	let mut controller = C::ContentController::new();
	let mut state = controller.new_state();

	window.set_visible(true);
	window.set_maximized(true);

	let mut last_frame = Instant::now();

	event_loop.run(move |event, _, control_flow| {
		platform.handle_event(imgui.io_mut(), &window, &event);
		match event {
			Event::NewEvents(_) => {
				let now = Instant::now();
				imgui.io_mut().update_delta_time(now - last_frame);
				last_frame = now;
			}
			Event::WindowEvent {
				event: WindowEvent::CloseRequested,
				..
			} => *control_flow = ControlFlow::Exit,

			Event::WindowEvent {
				event: WindowEvent::Resized(size),
				..
			} => {
				camera.resize(size.width as f32 / size.height as f32);
				render.resize(size.width, size.height);
			}
			Event::WindowEvent {
				event: WindowEvent::ScaleFactorChanged { new_inner_size, .. },
				..
			} => {
				camera.resize(new_inner_size.width as f32 / new_inner_size.height as f32);
				render.resize(new_inner_size.width, new_inner_size.height);
			}

			Event::WindowEvent {
				event: WindowEvent::MouseInput { state, button, .. },
				..
			} => {
				camera.mouse_button(state, button, &window);
			}
			Event::DeviceEvent {
				event: DeviceEvent::Key(keyboard_input),
				..
			} => {
				camera.keyboard_input(keyboard_input);
			}
			Event::DeviceEvent {
				event: DeviceEvent::MouseMotion { delta },
				..
			} => {
				camera.mouse_move(delta.0 as f32, delta.1 as f32);
			}

			Event::MainEventsCleared => {
				let scene = controller.render(&state, render.get_interface());
				render.render(camera.camera_view(), scene);
			}
			_ => (),
		};
	})
}
