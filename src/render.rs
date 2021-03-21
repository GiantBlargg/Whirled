use std::{borrow::Borrow, iter};

use gfx_hal::{
	adapter::Adapter,
	command::{
		ClearColor, ClearValue, CommandBuffer, CommandBufferFlags, Level, RenderAttachmentInfo,
		SubpassContents,
	},
	device::Device,
	format::{ChannelType, Format},
	image::{Extent, Layout},
	pass::{Attachment, AttachmentOps, SubpassDesc},
	pool::{CommandPool, CommandPoolCreateFlags},
	prelude::{CommandQueue, PhysicalDevice, QueueFamily},
	pso::{Rect, Viewport},
	queue::QueueGroup,
	window::{Extent2D, PresentationSurface, Surface, SwapchainConfig},
	Backend, Features, Instance,
};

pub struct Render<B: Backend> {
	instance: B::Instance,
	device: B::Device,
	surface: B::Surface,
	adapter: Adapter<B>,
	queue_group: QueueGroup<B>,

	command_pool: B::CommandPool,
	command_buffer: B::CommandBuffer,
	surface_colour_format: Format,
	render_pass: B::RenderPass,
	submission_complete_fence: B::Fence,
	rendering_complete_semaphore: B::Semaphore,
	surface_extent: Extent2D,
}

impl<B: Backend> Render<B> {
	pub fn new(window: &impl raw_window_handle::HasRawWindowHandle) -> Self {
		let instance = B::Instance::create("Whirled", 1).unwrap();

		let surface = unsafe { instance.create_surface(window).unwrap() };

		let adapter = instance.enumerate_adapters().remove(0);

		let queue_family = adapter
			.queue_families
			.iter()
			.find(|family| {
				surface.supports_queue_family(family) && family.queue_type().supports_graphics()
			})
			.unwrap();

		let mut gpu = unsafe {
			adapter
				.physical_device
				.open(&[(queue_family, &[1.0])], Features::empty())
				.unwrap()
		};

		let device = gpu.device;
		let queue_group = gpu.queue_groups.pop().unwrap();

		let mut command_pool = unsafe {
			device
				.create_command_pool(queue_group.family, CommandPoolCreateFlags::empty())
				.unwrap()
		};

		let command_buffer = unsafe { command_pool.allocate_one(Level::Primary) };

		let supported_formats = surface
			.supported_formats(&adapter.physical_device)
			.unwrap_or(vec![]);

		let fallback_format = *supported_formats.get(0).unwrap_or(&Format::Rgba8Srgb);

		let surface_colour_format = supported_formats
			.into_iter()
			.find(|format| format.base_format().1 == ChannelType::Srgb)
			.unwrap_or(fallback_format);

		let colour_attachment = Attachment {
			format: Some(surface_colour_format),
			samples: 1,
			ops: AttachmentOps::INIT,
			stencil_ops: AttachmentOps::DONT_CARE,
			layouts: Layout::Undefined..Layout::Present,
		};

		let subpass = SubpassDesc {
			colors: &[(0, Layout::ColorAttachmentOptimal)],
			depth_stencil: None,
			inputs: &[],
			resolves: &[],
			preserves: &[],
		};

		let render_pass = unsafe {
			device
				.create_render_pass(
					iter::once(colour_attachment),
					iter::once(subpass),
					iter::empty(),
				)
				.unwrap()
		};

		let submission_complete_fence = device.create_fence(true).unwrap();
		let rendering_complete_semaphore = device.create_semaphore().unwrap();

		Self {
			instance,
			surface,
			device,
			adapter,
			queue_group,
			command_pool,
			command_buffer,
			surface_colour_format,
			render_pass,
			submission_complete_fence,
			rendering_complete_semaphore,
			surface_extent: Extent2D {
				width: 0,
				height: 0,
			},
		}
	}
	pub fn resize(self: &mut Self, extent: Extent2D) {
		self.surface_extent = extent;
	}
	pub fn render(self: &mut Self) {
		unsafe {
			self.device
				.wait_for_fence(&self.submission_complete_fence, !0)
				.unwrap();
			self.device
				.reset_fence(&mut self.submission_complete_fence)
				.unwrap();
			self.command_pool.reset(false);
		}

		let caps = self.surface.capabilities(&self.adapter.physical_device);

		let swapchain_config =
			SwapchainConfig::from_caps(&caps, self.surface_colour_format, self.surface_extent);

		self.surface_extent = swapchain_config.extent;

		let fat = swapchain_config.framebuffer_attachment();

		unsafe {
			self.surface
				.configure_swapchain(&self.device, swapchain_config)
				.unwrap();
		}

		let framebuffer = unsafe {
			self.device
				.create_framebuffer(
					&self.render_pass,
					iter::once(fat),
					Extent {
						width: self.surface_extent.width,
						height: self.surface_extent.height,
						depth: 1,
					},
				)
				.unwrap()
		};

		let surface_image = match unsafe { self.surface.acquire_image(0) } {
			Ok((image, _)) => image,
			Err(_) => {
				return;
			}
		};

		let viewport = Viewport {
			rect: Rect {
				x: 0,
				y: 0,
				w: self.surface_extent.width as i16,
				h: self.surface_extent.height as i16,
			},
			depth: 0.0..1.0,
		};

		unsafe {
			self.command_buffer
				.begin_primary(CommandBufferFlags::ONE_TIME_SUBMIT);

			self.command_buffer
				.set_viewports(0, iter::once(viewport.clone()));
			self.command_buffer
				.set_scissors(0, iter::once(viewport.rect));

			self.command_buffer.begin_render_pass(
				&self.render_pass,
				&framebuffer,
				viewport.rect,
				iter::once(RenderAttachmentInfo {
					image_view: surface_image.borrow(),
					clear_value: ClearValue {
						color: ClearColor {
							float32: [0.0, 0.3, 0.8, 1.0],
						},
					},
				}),
				SubpassContents::Inline,
			);

			self.command_buffer.end_render_pass();

			self.command_buffer.finish();
		}

		unsafe {
			self.queue_group.queues[0].submit(
				iter::once(&self.command_buffer),
				iter::empty(),
				iter::once(&self.rendering_complete_semaphore),
				Some(&mut self.submission_complete_fence),
			);
			let result = self.queue_group.queues[0].present(
				&mut self.surface,
				surface_image,
				Some(&mut self.rendering_complete_semaphore),
			);
			result.unwrap();
		}
	}
}
