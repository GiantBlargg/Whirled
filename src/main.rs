mod render;

use gfx_hal::window::Extent2D;
use winit::{
	event::{Event, WindowEvent},
	event_loop::{ControlFlow, EventLoop},
	window::WindowBuilder,
};

use crate::render::Render;

fn main() {
	env_logger::Builder::from_env("WHIRLED_LOG")
		.filter(None, log::LevelFilter::Debug)
		.init();

	let event_loop = EventLoop::new();

	let window = WindowBuilder::new()
		.with_title("Whirled")
		.with_visible(false)
		.build(&event_loop)
		.unwrap();

	let mut render: Render<gfx_backend_vulkan::Backend> = Render::new(&window);

	event_loop.run(move |event, _, control_flow| match event {
		Event::NewEvents(winit::event::StartCause::Init) => {
			window.set_visible(true);
			window.set_maximized(true);
		}
		Event::WindowEvent { event, .. } => match event {
			WindowEvent::CloseRequested => *control_flow = ControlFlow::Exit,
			WindowEvent::Resized(size) => render.resize(Extent2D {
				width: size.width,
				height: size.height,
			}),
			WindowEvent::ScaleFactorChanged { new_inner_size, .. } => render.resize(Extent2D {
				width: new_inner_size.width,
				height: new_inner_size.width,
			}),
			_ => (),
		},
		Event::MainEventsCleared => render.render(),
		_ => (),
	})
}