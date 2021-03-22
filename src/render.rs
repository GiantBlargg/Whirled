use std::{iter, mem::ManuallyDrop};

use gfx_hal::{
	command,
	device::Device,
	format::{self, Format},
	image::{Extent, FramebufferAttachment, Layout, ViewCapabilities},
	pool::CommandPool,
	queue::{CommandQueue, QueueFamily},
	window::{Extent2D, PresentationSurface, Surface, DEFAULT_USAGE},
	Backend, Instance,
};

struct PerFrame<B: Backend> {
	command_pool: B::CommandPool,
	command_buffer: B::CommandBuffer,
	submission_complete_fence: B::Fence,
	rendering_complete_semaphore: B::Semaphore,
}

pub struct Render<B: Backend> {
	surface: ManuallyDrop<B::Surface>,
	render_pass: ManuallyDrop<B::RenderPass>,
	surface_colour_format: Format,
	surface_extent: Extent2D,
	reconfigure_swapchain: bool,
	framebuffer: ManuallyDrop<B::Framebuffer>,

	per_frame: Vec<PerFrame<B>>,
	frame: usize,

	device: B::Device,
	queue_group: gfx_hal::queue::QueueGroup<B>,
	adapter: gfx_hal::adapter::Adapter<B>,
	instance: B::Instance,
}

impl<B: Backend> Drop for Render<B> {
	fn drop(&mut self) {
		self.device.wait_idle().unwrap();
		self.resize_per_frame(0);
		unsafe {
			self.device
				.destroy_render_pass(ManuallyDrop::take(&mut self.render_pass));
			self.device
				.destroy_framebuffer(ManuallyDrop::take(&mut self.framebuffer));
			let mut surface = ManuallyDrop::take(&mut self.surface);
			surface.unconfigure_swapchain(&self.device);
			self.instance.destroy_surface(surface);
		}
	}
}

impl<B: Backend> Render<B> {
	pub fn new(window: &impl raw_window_handle::HasRawWindowHandle) -> Self {
		let instance = B::Instance::create("Whirled", 1).unwrap();

		let surface = unsafe { instance.create_surface(window).unwrap() };

		let adapter = instance.enumerate_adapters().remove(0);

		let (device, queue_group) = {
			let queue_family = adapter
				.queue_families
				.iter()
				.find(|family| {
					surface.supports_queue_family(family) && family.queue_type().supports_graphics()
				})
				.unwrap();

			let mut gpu = unsafe {
				use gfx_hal::adapter::PhysicalDevice;
				adapter
					.physical_device
					.open(&[(queue_family, &[1.0])], gfx_hal::Features::empty())
					.unwrap()
			};
			(gpu.device, gpu.queue_groups.pop().unwrap())
		};

		let surface_colour_format = {
			let supported_formats = surface
				.supported_formats(&adapter.physical_device)
				.unwrap_or(vec![]);

			let fallback_format = *supported_formats.get(0).unwrap_or(&Format::Rgba8Srgb);

			supported_formats
				.into_iter()
				.find(|format| format.base_format().1 == format::ChannelType::Srgb)
				.unwrap_or(fallback_format)
		};

		let render_pass = {
			use gfx_hal::pass::{self, AttachmentOps};

			let colour_attachment = pass::Attachment {
				format: Some(surface_colour_format),
				samples: 1,
				ops: AttachmentOps::INIT,
				stencil_ops: AttachmentOps::DONT_CARE,
				layouts: Layout::Undefined..Layout::Present,
			};

			let subpass = pass::SubpassDesc {
				colors: &[(0, Layout::ColorAttachmentOptimal)],
				depth_stencil: None,
				inputs: &[],
				resolves: &[],
				preserves: &[],
			};

			unsafe {
				device
					.create_render_pass(
						iter::once(colour_attachment),
						iter::once(subpass),
						iter::empty(),
					)
					.unwrap()
			}
		};

		let framebuffer = unsafe {
			device
				.create_framebuffer(
					&render_pass,
					iter::once(FramebufferAttachment {
						usage: DEFAULT_USAGE,
						view_caps: ViewCapabilities::empty(),
						format: surface_colour_format,
					}),
					Extent {
						width: 1,
						height: 1,
						depth: 1,
					},
				)
				.unwrap()
		};

		Self {
			instance,
			surface: ManuallyDrop::new(surface),
			device,
			adapter,
			queue_group,
			surface_colour_format,
			render_pass: ManuallyDrop::new(render_pass),
			surface_extent: Extent2D {
				width: 0,
				height: 0,
			},
			reconfigure_swapchain: true,
			framebuffer: ManuallyDrop::new(framebuffer),
			per_frame: Vec::new(),
			frame: 0,
		}
	}
	pub fn resize(&mut self, extent: Extent2D) {
		self.surface_extent = extent;
		self.reconfigure_swapchain = true;
	}
	fn resize_per_frame(&mut self, image_count: usize) {
		use std::cmp::Ordering;

		match image_count.cmp(&(self.per_frame.len())) {
			Ordering::Greater => {
				let additional = image_count - self.per_frame.len();
				self.per_frame.reserve_exact(additional);
				for _ in 0..additional {
					let mut command_pool = unsafe {
						self.device
							.create_command_pool(
								self.queue_group.family,
								gfx_hal::pool::CommandPoolCreateFlags::empty(),
							)
							.unwrap()
					};

					let command_buffer =
						unsafe { command_pool.allocate_one(command::Level::Primary) };

					let submission_complete_fence = self.device.create_fence(true).unwrap();
					let rendering_complete_semaphore = self.device.create_semaphore().unwrap();

					self.per_frame.push(PerFrame {
						command_pool,
						command_buffer,
						submission_complete_fence,
						rendering_complete_semaphore,
					});
				}
			}
			Ordering::Equal => {}
			Ordering::Less => {
				let to_delete: Vec<PerFrame<B>> = self.per_frame.drain(image_count..).collect();
				to_delete.into_iter().for_each(|per_frame| unsafe {
					self.device
						.destroy_fence(per_frame.submission_complete_fence);
					self.device
						.destroy_semaphore(per_frame.rendering_complete_semaphore);
					self.device.destroy_command_pool(per_frame.command_pool);
				});
			}
		}
	}
	pub fn render(&mut self) {
		if self.reconfigure_swapchain {
			self.reconfigure_swapchain = false;

			self.device.wait_idle().unwrap();

			let caps = self.surface.capabilities(&self.adapter.physical_device);

			let swapchain_config = gfx_hal::window::SwapchainConfig::from_caps(
				&caps,
				self.surface_colour_format,
				self.surface_extent,
			);

			self.surface_extent = swapchain_config.extent;

			let image_count = swapchain_config.image_count;

			unsafe {
				self.surface
					.configure_swapchain(&self.device, swapchain_config)
					.unwrap();
			}
			unsafe {
				self.device
					.destroy_framebuffer(ManuallyDrop::take(&mut self.framebuffer));
			}
			self.framebuffer = ManuallyDrop::new(unsafe {
				self.device
					.create_framebuffer(
						&self.render_pass,
						iter::once(FramebufferAttachment {
							usage: DEFAULT_USAGE,
							view_caps: ViewCapabilities::empty(),
							format: self.surface_colour_format,
						}),
						Extent {
							width: self.surface_extent.width,
							height: self.surface_extent.height,
							depth: 1,
						},
					)
					.unwrap()
			});

			self.resize_per_frame(image_count as usize);
		}

		let surface_image = match unsafe { self.surface.acquire_image(!0) } {
			Ok((image, _)) => image,
			Err(_) => {
				self.reconfigure_swapchain = true;
				return;
			}
		};

		let frame = &mut self.per_frame[self.frame];

		unsafe {
			self.device
				.wait_for_fence(&frame.submission_complete_fence, !0)
				.unwrap();
			self.device
				.reset_fence(&mut frame.submission_complete_fence)
				.unwrap();
			frame.command_pool.reset(false);
		}

		let viewport = {
			use gfx_hal::pso;
			pso::Viewport {
				rect: pso::Rect {
					x: 0,
					y: 0,
					w: self.surface_extent.width as i16,
					h: self.surface_extent.height as i16,
				},
				depth: 0.0..1.0,
			}
		};

		let command_buffer = &mut frame.command_buffer;

		unsafe {
			use command::CommandBuffer;

			command_buffer.begin_primary(command::CommandBufferFlags::ONE_TIME_SUBMIT);

			command_buffer.set_viewports(0, iter::once(viewport.clone()));
			command_buffer.set_scissors(0, iter::once(viewport.rect));

			command_buffer.begin_render_pass(
				&self.render_pass,
				&self.framebuffer,
				viewport.rect,
				iter::once(command::RenderAttachmentInfo {
					image_view: {
						use std::borrow::Borrow;
						surface_image.borrow()
					},
					clear_value: command::ClearValue {
						color: command::ClearColor {
							float32: [0.0, 0.3, 0.8, 1.0],
						},
					},
				}),
				command::SubpassContents::Inline,
			);

			command_buffer.end_render_pass();

			command_buffer.finish();
		}

		unsafe {
			self.queue_group.queues[0].submit(
				iter::once(&frame.command_buffer),
				iter::empty(),
				iter::once(&frame.rendering_complete_semaphore),
				Some(&mut frame.submission_complete_fence),
			);
			let result = self.queue_group.queues[0].present(
				&mut self.surface,
				surface_image,
				Some(&mut frame.rendering_complete_semaphore),
			);
			if result.is_err() {
				self.reconfigure_swapchain = true;
			}
		}

		self.frame = (self.frame + 1) % self.per_frame.len();
	}
}
