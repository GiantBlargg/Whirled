pub mod render;

use gfx_hal::{window::Extent2D, Backend};
use winit::{
	event::{Event, WindowEvent},
	event_loop::{ControlFlow, EventLoop},
	window::WindowBuilder,
};

use self::render::Render;

pub trait Content<B: Backend>: 'static {
	type TrackedState;
	fn new() -> Self;
	fn new_state() -> Self::TrackedState;
	fn render(&self, state: &Self::TrackedState, render: &mut Render<B>);
}

pub fn whirled<B: Backend, C: Content<B>>() -> ! {
	let event_loop = EventLoop::new();

	let window = WindowBuilder::new()
		.with_title("Whirled")
		.with_visible(false)
		.build(&event_loop)
		.unwrap();

	let mut render = Render::<B>::new(&window);

	let content = C::new();
	let state = C::new_state();

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
		Event::MainEventsCleared => content.render(&state, &mut render),
		_ => (),
	})
}
