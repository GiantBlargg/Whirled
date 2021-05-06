use std::{iter, mem::ManuallyDrop};

use gfx_hal::{
	command,
	device::Device,
	format::{self, Format},
	image::{Extent, FramebufferAttachment, Layout, ViewCapabilities},
	pool::CommandPool,
	queue::{Queue, QueueFamily},
	window::{Extent2D, PresentationSurface, Surface, DEFAULT_USAGE},
	Backend, Instance,
};

use super::container::{Container, ContainerGAT};

struct PerFrame<B: Backend> {
	command_pool: B::CommandPool,
	command_buffer: B::CommandBuffer,
	submission_fence: B::Fence,
	rendering_semaphore: B::Semaphore,
}

type Model<B> = Vec<Mesh<B>>;

struct Mesh<B: Backend> {
	pipeline: B::GraphicsPipeline,
	vertex_count: u32,
}

pub struct Render<B: Backend, C: ContainerGAT> {
	models: C::Container<Model<B>>,

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

impl<B: Backend, C: ContainerGAT> Drop for Render<B, C> {
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

impl<B: Backend, C: ContainerGAT> Render<B, C> {
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
			models: C::Container::new(),

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

					let submission_fence = self.device.create_fence(true).unwrap();
					let rendering_semaphore = self.device.create_semaphore().unwrap();

					self.per_frame.push(PerFrame {
						command_pool,
						command_buffer,
						submission_fence,
						rendering_semaphore,
					});
				}
			}
			Ordering::Equal => {}
			Ordering::Less => {
				for per_frame in self.per_frame.drain(image_count..) {
					unsafe {
						self.device.destroy_fence(per_frame.submission_fence);
						self.device.destroy_semaphore(per_frame.rendering_semaphore);
						self.device.destroy_command_pool(per_frame.command_pool);
					}
				}
			}
		}
	}
}

impl<B: Backend, C: ContainerGAT> super::RenderInterface for Render<B, C> {
	type ModelHandle = C::Handle;
	fn add_model(&mut self, _model: super::ModelDef) -> Self::ModelHandle {
		let pipeline = {
			use gfx_hal::pso::{self, EntryPoint};

			let pipeline_layout = unsafe {
				self.device
					.create_pipeline_layout(iter::empty(), iter::empty())
					.unwrap()
			};

			let vertex_shader_module = unsafe {
				self.device
					.create_shader_module(&shaders::TEST_VERT)
					.unwrap()
			};
			let fragment_shader_module = unsafe {
				self.device
					.create_shader_module(&shaders::TEST_FRAG)
					.unwrap()
			};
			let desc = pso::GraphicsPipelineDesc {
				label: None,
				primitive_assembler: pso::PrimitiveAssemblerDesc::Vertex {
					buffers: &[],
					attributes: &[],
					input_assembler: pso::InputAssemblerDesc::new(pso::Primitive::TriangleList),
					vertex: EntryPoint {
						entry: "main",
						module: &vertex_shader_module,
						specialization: pso::Specialization::default(),
					},
					tessellation: None,
					geometry: None,
				},
				rasterizer: pso::Rasterizer {
					polygon_mode: pso::PolygonMode::Fill,
					cull_face: pso::Face::NONE,
					front_face: pso::FrontFace::Clockwise,
					depth_clamping: false,
					depth_bias: None,
					conservative: false,
					line_width: pso::State::Static(1.0),
				},
				fragment: Some(EntryPoint {
					entry: "main",
					module: &fragment_shader_module,
					specialization: pso::Specialization::default(),
				}),
				blender: pso::BlendDesc {
					logic_op: None,
					targets: vec![pso::ColorBlendDesc::EMPTY],
				},
				depth_stencil: pso::DepthStencilDesc::default(),
				multisampling: None,
				baked_states: pso::BakedStates::default(),
				layout: &pipeline_layout,
				subpass: gfx_hal::pass::Subpass {
					index: 0,
					main_pass: &*self.render_pass,
				},
				flags: pso::PipelineCreationFlags::empty(),
				parent: pso::BasePipeline::None,
			};
			let pipeline = unsafe { self.device.create_graphics_pipeline(&desc, None) };
			unsafe {
				self.device.destroy_pipeline_layout(pipeline_layout);
				self.device.destroy_shader_module(vertex_shader_module);
				self.device.destroy_shader_module(fragment_shader_module);
			}
			pipeline.unwrap()
		};
		self.models.insert(vec![Mesh {
			pipeline,
			vertex_count: 3,
		}])
	}
	fn render(&mut self, models: impl Iterator<Item = super::ModelInstance<Self>>) {
		if self.reconfigure_swapchain {
			self.reconfigure_swapchain = false;

			self.device.wait_idle().unwrap();

			let caps = self.surface.capabilities(&self.adapter.physical_device);

			let swapchain_config = gfx_hal::window::SwapchainConfig::from_caps(
				&caps,
				self.surface_colour_format,
				self.surface_extent,
			)
			.with_present_mode(gfx_hal::window::PresentMode::FIFO);

			self.surface_extent = swapchain_config.extent;

			unsafe {
				self.device
					.destroy_framebuffer(ManuallyDrop::take(&mut self.framebuffer));
			}
			self.framebuffer = ManuallyDrop::new(unsafe {
				self.device
					.create_framebuffer(
						&self.render_pass,
						iter::once(swapchain_config.framebuffer_attachment()),
						Extent {
							width: self.surface_extent.width,
							height: self.surface_extent.height,
							depth: 1,
						},
					)
					.unwrap()
			});

			self.resize_per_frame(swapchain_config.image_count as usize);

			unsafe {
				self.surface
					.configure_swapchain(&self.device, swapchain_config)
					.unwrap();
			}
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
				.wait_for_fence(&frame.submission_fence, !0)
				.unwrap();
			self.device
				.reset_fence(&mut frame.submission_fence)
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

		let cmd_buffer = &mut frame.command_buffer;

		unsafe {
			use command::CommandBuffer;

			cmd_buffer.begin_primary(command::CommandBufferFlags::ONE_TIME_SUBMIT);

			cmd_buffer.set_viewports(0, iter::once(viewport.clone()));
			cmd_buffer.set_scissors(0, iter::once(viewport.rect));

			cmd_buffer.begin_render_pass(
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

			for model in models {
				let model = self.models.get(&model.model).unwrap();
				for mesh in model {
					cmd_buffer.bind_graphics_pipeline(&mesh.pipeline);
					cmd_buffer.draw(0..mesh.vertex_count, 0..1);
				}
			}

			cmd_buffer.end_render_pass();

			cmd_buffer.finish();
		}

		unsafe {
			self.queue_group.queues[0].submit(
				iter::once(&frame.command_buffer),
				iter::empty(),
				iter::once(&frame.rendering_semaphore),
				Some(&mut frame.submission_fence),
			);
			let result = self.queue_group.queues[0].present(
				&mut self.surface,
				surface_image,
				Some(&mut frame.rendering_semaphore),
			);
			if result.is_err() {
				self.reconfigure_swapchain = true;
			}
		}

		self.frame = (self.frame + 1) % self.per_frame.len();
	}
}
