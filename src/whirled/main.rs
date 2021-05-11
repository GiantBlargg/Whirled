use winit::{
	event::{Event, WindowEvent},
	event_loop::{ControlFlow, EventLoop},
	window::WindowBuilder,
};

use super::{Content, ContentController, WhirledRender};

pub fn whirled<Render: WhirledRender, C: Content>() {
	let event_loop = EventLoop::new();

	let window = WindowBuilder::new()
		.with_title("Whirled")
		.with_visible(false)
		.build(&event_loop)
		.unwrap();

	let mut render = Render::new(&window);

	let mut controller = C::ContentController::new();
	let mut state = controller.new_state();

	event_loop.run(move |event, _, control_flow| match event {
		Event::NewEvents(winit::event::StartCause::Init) => {
			window.set_visible(true);
			window.set_maximized(true);
		}
		Event::WindowEvent { event, .. } => match event {
			WindowEvent::CloseRequested => *control_flow = ControlFlow::Exit,
			WindowEvent::Resized(size) => render.resize(size.width, size.height),
			WindowEvent::ScaleFactorChanged { new_inner_size, .. } => {
				render.resize(new_inner_size.width, new_inner_size.width)
			}
			_ => (),
		},
		Event::MainEventsCleared => {
			let scene = controller.render(&state, render.get_interface());
			render.render(scene);
		}
		_ => (),
	})
}
