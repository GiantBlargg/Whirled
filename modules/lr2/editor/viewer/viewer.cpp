#include "viewer.hpp"

#include "scene/3d/camera_3d.h"
#include "scene/3d/collision_shape_3d.h"
#include "scene/3d/light_3d.h"
#include "scene/gui/subviewport_container.h"
#include "scene/resources/primitive_meshes.h"

#include "gizmo.hpp"
#include "lr2/assets/ifl.hpp"
#include "lr2/assets/image_asset_loader.hpp"
#include "lr2/assets/mdl2.hpp"
#include "lr2/assets/mesh_shape.hpp"
#include "lr2/assets/tdf.hpp"

void Viewer::update_cameras() {
	bg_camera->set_basis(camera->get_basis());

	for (Gizmo* g : gizmos) {
		g->update_camera(camera->get_transform());
	}
}

void Viewer::update_gizmos(WRL::EntryID selected) {
	for (Gizmo* g : gizmos) {
		g->queue_free();
	}
	gizmos.clear();
	if (selected) {
		auto& model = wrl->get_entry_format(selected).model;
		if (model) {
			if (model.position != "") {
				gizmos.append(memnew(Trans1DGizmo(selected, model.position, GizmoDir::X_AXIS)));
				gizmos.append(memnew(Trans1DGizmo(selected, model.position, GizmoDir::Y_AXIS)));
				gizmos.append(memnew(Trans1DGizmo(selected, model.position, GizmoDir::Z_AXIS)));

				if (model.rotation != "") {
					gizmos.append(memnew(RotateGizmo(selected, model.position, model.rotation, GizmoDir::X_AXIS)));
					gizmos.append(memnew(RotateGizmo(selected, model.position, model.rotation, GizmoDir::Y_AXIS)));
					gizmos.append(memnew(RotateGizmo(selected, model.position, model.rotation, GizmoDir::Z_AXIS)));
				}
			}
		}
	}
	for (Gizmo* g : gizmos) {
		root->add_child(g);
		g->wrl_connect(wrl);
	}
	update_cameras();
}

void Viewer::input(const Ref<InputEvent>& p_event) {
	Ref<InputEventKey> k = p_event;
	if (mode == Mode::FPS && k.is_valid())
		get_viewport()->set_input_as_handled();
}

void Viewer::gui_input(const Ref<InputEvent>& p_event) {
	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid()) {
		if (mode == Mode::Default && mb->get_button_index() == MouseButton::LEFT && mb->is_pressed()) {
			const Vector2 cursor_pos = get_local_mouse_position();
			const Vector3 ray_origin = camera->project_ray_origin(cursor_pos);
			const Vector3 ray_normal = camera->project_ray_normal(cursor_pos);

			if (current_gizmo) {
				mode = Mode::Gizmo;
				current_gizmo->interact(ray_origin, ray_normal, true);
			} else {
				// Pick object under cursor
				PhysicsDirectSpaceState3D::RayParameters ray_params{
					.collide_with_bodies = true,
					.collide_with_areas = true,
					.hit_from_inside = true,
					.hit_back_faces = true};
				ray_params.from = ray_origin;
				ray_params.to = ray_origin + ray_normal * camera->get_far();
				ray_params.collision_mask = LayerProps | LayerTerrain;

				PhysicsDirectSpaceState3D::RayResult rr;
				bool found = PhysicsServer3D::get_singleton()
								 ->space_get_direct_state(camera->get_world_3d()->get_space())
								 ->intersect_ray(ray_params, rr);

				if (!found) {
					const Vector3 ray_origin = bg_camera->project_ray_origin(cursor_pos);
					const Vector3 ray_normal = bg_camera->project_ray_normal(cursor_pos);
					ray_params.from = ray_origin;
					ray_params.to = ray_origin + ray_normal * bg_camera->get_far();
					ray_params.collision_mask = LayerSkyBox;

					found = PhysicsServer3D::get_singleton()
								->space_get_direct_state(bg_camera->get_world_3d()->get_space())
								->intersect_ray(ray_params, rr);
				}

				if (found) {
					WRL::EntryID found_entry;

					for (const auto& i : instances) {
						if (i.value.collider == rr.collider) {
							found_entry = i.key;
							break;
						}
					}

					if (found_entry) {
						wrl->submit_change(
							WRL::Change{.select_changed = true, .select = {wrl->get_index(found_entry), found_entry}});
					}
				}
			}
		} else if (mode == Mode::Default || mode == Mode::FPS) {
			if (mb->get_button_index() == MouseButton::RIGHT) {
				mode = mb->is_pressed() ? Mode::FPS : Mode::Default;
				Input::get_singleton()->set_mouse_mode(
					mb->is_pressed() ? Input::MOUSE_MODE_CAPTURED : Input::MOUSE_MODE_VISIBLE);
			}
		} else if (mode == Mode::Gizmo && mb->get_button_index() == MouseButton::LEFT && !mb->is_pressed()) {

			const Vector2 cursor_pos = get_local_mouse_position();
			const Vector3 ray_origin = camera->project_ray_origin(cursor_pos);
			const Vector3 ray_normal = camera->project_ray_normal(cursor_pos);
			current_gizmo->interact(ray_origin, ray_normal);

			mode = Mode::Default;
		}
	}

	Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid()) {
		if (mode == Mode::Default) {
			// Check if hovering over gizmo
			const Vector2 cursor_pos = get_local_mouse_position();

			PhysicsDirectSpaceState3D::RayParameters ray_params{
				.collide_with_bodies = true,
				.collide_with_areas = true,
				.hit_from_inside = true,
				.hit_back_faces = true};

			const Vector3 ray_origin = camera->project_ray_origin(cursor_pos);
			const Vector3 ray_normal = camera->project_ray_normal(cursor_pos);
			ray_params.from = ray_origin;
			ray_params.to = ray_origin + ray_normal * camera->get_far();
			ray_params.collision_mask = LayerGizmo;

			PhysicsDirectSpaceState3D::RayResult rr;
			bool found = PhysicsServer3D::get_singleton()
							 ->space_get_direct_state(camera->get_world_3d()->get_space())
							 ->intersect_ray(ray_params, rr);

			Gizmo* new_gizmo = nullptr;
			if (found) {
				new_gizmo = dynamic_cast<Gizmo*>(dynamic_cast<Node*>(rr.collider)->get_parent());
			}

			if (new_gizmo != current_gizmo) {
				if (current_gizmo)
					current_gizmo->mouse_over(false);

				current_gizmo = new_gizmo;

				if (current_gizmo)
					current_gizmo->mouse_over(true);
			}

		} else if (mode == Mode::FPS) {
			Vector3 old_rot = camera->get_rotation();
			Vector2 rel = mm->get_relative() * look_speed;
			Vector3 new_rot(CLAMP(old_rot.x - rel.y, -Math_PI / 2, Math_PI / 2), (old_rot.y - rel.x), 0);
			camera->set_rotation(new_rot);
			update_cameras();
		} else if (mode == Mode::Gizmo) {
			const Vector2 cursor_pos = get_local_mouse_position();
			const Vector3 ray_origin = camera->project_ray_origin(cursor_pos);
			const Vector3 ray_normal = camera->project_ray_normal(cursor_pos);
			current_gizmo->interact(ray_origin, ray_normal);
		}
	}
}

void Viewer::_notification(int p_what) {
	if (p_what == NOTIFICATION_PROCESS) {
		if (mode == Mode::FPS) {
			Vector3 movement;

			Input* i = Input::get_singleton();

			if (i->is_key_pressed(Key::W))
				movement.z--;
			if (i->is_key_pressed(Key::S))
				movement.z++;
			if (i->is_key_pressed(Key::A))
				movement.x--;
			if (i->is_key_pressed(Key::D))
				movement.x++;
			if (i->is_key_pressed(Key::Q))
				movement.y--;
			if (i->is_key_pressed(Key::E))
				movement.y++;

			if (i->is_key_pressed(Key::SHIFT))
				movement *= 5;
			if (i->is_key_pressed(Key::CTRL))
				movement /= 5;

			movement *= get_process_delta_time() * move_speed;
			camera->translate(movement);
			update_cameras();
		}

		for (WRL::EntryID entry : pending) {
			Instance& i = instances[entry];
			Ref<Mesh> mesh = assets.try_get<Mesh>(i.model_path);
			if (mesh.is_valid()) {
				i.mesh_instance->set_mesh(mesh);
			}
			Ref<Shape3D> shape = assets.try_get<Shape3D>(i.model_path);
			if (shape.is_valid()) {
				i.collider = memnew(StaticBody3D);
				i.mesh_instance->add_child(i.collider);
				i.collider->set_collision_layer(i.mesh_instance->get_layer_mask());
				CollisionShape3D* cshape = memnew(CollisionShape3D);
				i.collider->add_child(cshape, true);
				cshape->set_shape(shape);
			}
			if (mesh.is_valid() && shape.is_valid()) {
				pending.erase(entry);
			}
		}
	}
}

void Viewer::_wrl_changed(const WRL::Change& change, bool) {
	for (const auto& r : change.removed) {
		if (instances.has(r.value)) {
			instances[r.value].mesh_instance->queue_free();
			instances.erase(r.value);
		}
		if (pending.has(r.value)) {
			pending.erase(r.value);
		}
	}

	for (const auto& a : change.added) {
		auto& model = wrl->get_entry_format(a.value).model;
		if (model) {
			auto mesh_instance = memnew(MeshInstance3D);
			static std::unordered_map<WRL::Format::Model::Type, Layer> layer_lookup = {
				{WRL::Format::Model::Type::Prop, LayerProps},
				{WRL::Format::Model::Type::Terrain, LayerTerrain},
				{WRL::Format::Model::Type::Skybox, LayerSkyBox},
			};
			mesh_instance->set_layer_mask(layer_lookup.at(model.type));
			root->add_child(mesh_instance);
			instances.insert(a.value, {.mesh_instance = mesh_instance});
		}
	}

	for (const auto& prop : change.propertyChanges) {
		WRL::EntryID entry = prop.key.first;
		if (!instances.has(entry))
			continue;

		auto& model = wrl->get_entry_format(entry).model;
		String prop_name = prop.key.second;
		if (prop_name == model.model) {
			const String& model = prop.value;
			if (instances[entry].model_path != model) {
				if (instances[entry].collider)
					instances[entry].collider->queue_free();
				instances[entry].model_path = model;
				pending.insert(entry);
			}
		}

		else if (prop_name == model.position || prop_name == model.rotation || prop_name == model.scale) {
			if (prop_name == model.position) {
				instances[entry].position = prop.value;
			} else if (prop_name == model.rotation) {
				instances[entry].rotation = prop.value;
			} else if (prop_name == model.scale) {
				instances[entry].scale = prop.value;
			}

			instances[entry].mesh_instance->set_transform(Transform3D(
				Basis(instances[entry].rotation).scaled(instances[entry].scale), instances[entry].position));
		}

		else if (model.uniforms.has(prop_name)) {
			instances[entry].mesh_instance->set_instance_shader_parameter(prop_name, prop.value);
		}
	}

	if (change.select_changed) {
		update_gizmos(change.select.second);
	}
}

Viewer::Viewer(const CustomFS& p_custom_fs) : custom_fs(p_custom_fs), assets(custom_fs) {
	assets.add_loader<ImageAssetLoader>();
	assets.add_loader<ImageTextureLoader>();
	assets.add_loader<IFLLoader>();
	assets.add_loader<MDL2Loader>();
	assets.add_loader<TDFLoader>();
	assets.add_loader<TDFMeshLoader>();
	assets.add_loader<MeshShapeLoader>();

	set_process(true);
	set_process_input(true);

	SubViewportContainer* viewport_container = memnew(SubViewportContainer);
	viewport_container->set_stretch(true);
	viewport_container->set_custom_minimum_size(Size2(2, 2));
	viewport_container->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	viewport_container->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	add_child(viewport_container);

	SubViewport* bg_viewport = memnew(SubViewport);
	bg_viewport->set_debug_draw(Viewport::DEBUG_DRAW_UNSHADED);
	viewport_container->add_child(bg_viewport);
	bg_camera = memnew(Camera3D);
	bg_camera->set_near(1);
	bg_camera->set_cull_mask(LayerSkyBox);
	bg_viewport->add_child(bg_camera);

	SubViewport* viewport = memnew(SubViewport);
	viewport_container->add_child(viewport);
	camera = memnew(Camera3D);
	camera->set_near(1);
	camera->set_cull_mask(LayerProps | LayerTerrain | LayerGizmo);
	viewport->add_child(camera);

	MeshInstance3D* skybox_viewer = memnew(MeshInstance3D);
	skybox_viewer->set_mesh(memnew(QuadMesh));
	Ref<Shader> skybox_viewer_shader = memnew(Shader);
	skybox_viewer_shader->set_code(R"(
shader_type spatial;
render_mode unshaded;
uniform sampler2D skybox : source_color;
void vertex() { POSITION = vec4(VERTEX.x * 2.0, VERTEX.y * -2.0, 1.0, 1.0); }
void fragment() { ALBEDO = texture(skybox, UV).rgb; }
)");
	Ref<ShaderMaterial> skybox_viewer_material = memnew(ShaderMaterial);
	skybox_viewer_material->set_shader(skybox_viewer_shader);
	skybox_viewer_material->set_shader_parameter("skybox", bg_viewport->get_texture());
	skybox_viewer->set_material_override(skybox_viewer_material);
	skybox_viewer->set_position(Vector3(0, 0, -2));
	camera->add_child(skybox_viewer);

	root = memnew(Node3D);
	root->set_scale({-1, 1, 1});
	add_child(root);

	DirectionalLight3D* l = memnew(DirectionalLight3D);
	l->rotate_x(-Math_PI / 4);
	add_child(l);
}