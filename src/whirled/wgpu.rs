use std::{borrow::Cow, mem::size_of};

use wgpu::{util::DeviceExt, Buffer};

use super::container::{Container, CounterMap};

struct Model {
	pipeline: wgpu::RenderPipeline,
	vertex_buffer: Buffer,
	index_buffer: Buffer,
	vertex_count: u32,
}

pub struct Render {
	surface: wgpu::Surface,
	surface_format: wgpu::TextureFormat,
	device: wgpu::Device,
	queue: wgpu::Queue,
	width: u32,
	height: u32,
	swapchain: Option<wgpu::SwapChain>,
	models: CounterMap<Model>,
}

impl super::WhirledRender for Render {
	fn new(window: &impl raw_window_handle::HasRawWindowHandle) -> Self {
		let instance = wgpu::Instance::new(wgpu::BackendBit::PRIMARY);

		let surface = unsafe { instance.create_surface(window) };

		let (adapter, device, queue) = async_std::task::block_on(async {
			let adapter = instance
				.request_adapter(&wgpu::RequestAdapterOptionsBase {
					power_preference: wgpu::PowerPreference::HighPerformance,
					compatible_surface: Some(&surface),
				})
				.await
				.unwrap();
			let (device, queue) = adapter
				.request_device(
					&wgpu::DeviceDescriptor {
						label: None,
						features: wgpu::Features::empty(),
						limits: wgpu::Limits::default(),
					},
					None,
				)
				.await
				.unwrap();
			(adapter, device, queue)
		});

		let surface_format = adapter.get_swap_chain_preferred_format(&surface).unwrap();

		Self {
			surface,
			surface_format,
			device,
			queue,
			width: 1,
			height: 1,
			swapchain: None,

			models: CounterMap::new(),
		}
	}
	fn resize(&mut self, width: u32, height: u32) {
		self.swapchain = None;
		self.width = width;
		self.height = height;
	}

	type RenderInterface = Self;

	fn get_interface(&mut self) -> &mut Self::RenderInterface {
		self
	}

	fn render(&mut self, scene: super::RenderScene<Self::RenderInterface>) {
		let swapchain = {
			if self.swapchain.is_none() {
				self.swapchain = Some(self.device.create_swap_chain(
					&self.surface,
					&wgpu::SwapChainDescriptor {
						usage: wgpu::TextureUsage::RENDER_ATTACHMENT,
						format: self.surface_format,
						width: self.width,
						height: self.height,
						present_mode: wgpu::PresentMode::Fifo,
					},
				));
			}
			self.swapchain.as_ref().unwrap()
		};

		let frame = match swapchain.get_current_frame() {
			Ok(frame) => frame,
			Err(err) => {
				match err {
					wgpu::SwapChainError::OutOfMemory => {
						panic!()
					}
					_ => self.swapchain = None,
				};
				return;
			}
		}
		.output;

		let mut cmd = self
			.device
			.create_command_encoder(&wgpu::CommandEncoderDescriptor { label: None });

		{
			let mut render_pass = cmd.begin_render_pass(&wgpu::RenderPassDescriptor {
				label: None,
				color_attachments: &[wgpu::RenderPassColorAttachment {
					view: &frame.view,
					resolve_target: None,
					ops: wgpu::Operations {
						load: wgpu::LoadOp::Clear(wgpu::Color {
							r: 0.0,
							g: 0.3,
							b: 0.8,
							a: 1.0,
						}),
						store: true,
					},
				}],
				depth_stencil_attachment: None,
			});
			for m in scene.models {
				let model = self.models.get(&m.model).unwrap();
				render_pass.set_pipeline(&model.pipeline);
				render_pass.set_vertex_buffer(0, model.vertex_buffer.slice(..));
				render_pass
					.set_index_buffer(model.index_buffer.slice(..), wgpu::IndexFormat::Uint16);
				render_pass.draw_indexed(0..model.vertex_count, 0, 0..1);
			}
		}

		self.queue.submit(std::iter::once(cmd.finish()));
	}
}

unsafe fn any_as_u8_slice<T: Sized>(p: &T) -> &[u8] {
	std::slice::from_raw_parts((p as *const T) as *const u8, size_of::<T>())
}

impl super::RenderInterface for Render {
	type ModelHandle = usize;

	fn add_model(&mut self, model: super::ModelDef) -> Self::ModelHandle {
		use glam::vec2;
		use wgpu::{BufferUsage, ShaderFlags, ShaderModuleDescriptor, ShaderSource};
		let pipeline = {
			let pipeline_layout =
				self.device
					.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
						label: None,
						bind_group_layouts: &[],
						push_constant_ranges: &[],
					});

			let vertex_shader = self.device.create_shader_module(&ShaderModuleDescriptor {
				label: None,
				source: ShaderSource::SpirV(Cow::Borrowed(&shaders::TEST2_VERT)),
				flags: ShaderFlags::empty(),
			});
			let fragment_shader = self.device.create_shader_module(&ShaderModuleDescriptor {
				label: None,
				source: ShaderSource::SpirV(Cow::Borrowed(&shaders::TEST_FRAG)),
				flags: ShaderFlags::empty(),
			});
			self.device
				.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
					label: None,
					layout: Some(&pipeline_layout),
					vertex: wgpu::VertexState {
						module: &vertex_shader,
						entry_point: "main",
						buffers: &[wgpu::VertexBufferLayout {
							array_stride: size_of::<glam::Vec2>() as wgpu::BufferAddress,
							step_mode: wgpu::InputStepMode::Vertex,
							attributes: &[wgpu::VertexAttribute {
								offset: 0,
								shader_location: 0,
								format: wgpu::VertexFormat::Float32x2,
							}],
						}],
					},
					primitive: wgpu::PrimitiveState {
						topology: wgpu::PrimitiveTopology::TriangleList,
						strip_index_format: None,
						front_face: wgpu::FrontFace::Cw,
						cull_mode: None,
						clamp_depth: false,
						polygon_mode: wgpu::PolygonMode::Fill,
						conservative: false,
					},
					depth_stencil: None,
					multisample: wgpu::MultisampleState {
						count: 1,
						mask: !0,
						alpha_to_coverage_enabled: false,
					},
					fragment: Some(wgpu::FragmentState {
						module: &fragment_shader,
						entry_point: "main",
						targets: &[wgpu::ColorTargetState {
							format: self.surface_format,
							blend: Some(wgpu::BlendState::REPLACE),
							write_mask: wgpu::ColorWrite::ALL,
						}],
					}),
				})
		};
		let vertex_buffer = self
			.device
			.create_buffer_init(&wgpu::util::BufferInitDescriptor {
				label: None,
				contents: unsafe {
					any_as_u8_slice(&[vec2(0.0, -0.5), vec2(0.5, 0.5), vec2(-0.5, 0.5)])
				},
				usage: wgpu::BufferUsage::VERTEX,
			});
		let index_buffer = self
			.device
			.create_buffer_init(&wgpu::util::BufferInitDescriptor {
				label: None,
				contents: unsafe { any_as_u8_slice(&([0, 1, 2] as [u16; 3])) },
				usage: BufferUsage::INDEX,
			});
		self.models.insert(Model {
			pipeline,
			vertex_buffer,
			index_buffer,
			vertex_count: 3,
		})
	}
}
