#include "viewer.h"

#include "../import/ifl.hpp"
#include "../import/image_asset_loader.hpp"
#include "../import/mdl2.hpp"
#include "modules/lr2/import/tdf.h"
#include "scene/3d/camera_3d.h"
#include "scene/3d/light_3d.h"
#include "scene/gui/subviewport_container.h"
#include "scene/resources/primitive_meshes.h"

void Viewer::input(const Ref<InputEvent>& p_event) {
	Ref<InputEventKey> k = p_event;
	if (right && k.is_valid())
		get_viewport()->set_input_as_handled();
}

void Viewer::gui_input(const Ref<InputEvent>& p_event) {
	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid()) {
		if (mb->get_button_index() == MouseButton::RIGHT) {
			right = mb->is_pressed();
			Input::get_singleton()->set_mouse_mode(
				mb->is_pressed() ? Input::MOUSE_MODE_CAPTURED : Input::MOUSE_MODE_VISIBLE);
		}
	}

	Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid()) {
		if (right) {
			Vector3 old_rot = camera->get_rotation();
			Vector2 rel = mm->get_relative() * look_speed;
			Vector3 new_rot(CLAMP(old_rot.x - rel.y, -Math_PI / 2, Math_PI / 2), (old_rot.y - rel.x), 0);
			camera->set_rotation(new_rot);
			bg_camera->set_rotation(new_rot);
		}
	}
}

void Viewer::_notification(int p_what) {
	if (p_what == NOTIFICATION_PROCESS) {
		if (right) {
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
		}

		for (auto p = pending.front(); p; p = p->next()) {
			int id = p->get();
			Instance i = instances[id];
			Ref<Mesh> mesh = assets.try_get<Mesh>(i.model_path);
			if (mesh.is_valid()) {
				i.mesh_instance->set_mesh(mesh);
				pending.erase(p);
			}
		}
	}
}

void Viewer::_wrl_changed(const WRL::Change& change, bool) {
	for (auto r = change.removed.front(); r; r = r.next()) {
		if (instances.has(r.value().id)) {
			instances[r.value().id].mesh_instance->queue_delete();
			instances.erase(r.value().id);
		}
	}

	for (auto a = change.added.front(); a; a = a.next()) {
		String type = wrl->get_entry_property(a.value(), "type");
		if (type == "cGeneralStatic" || type == "cGoldenBrick" || type == "cGeneralMobile" || type == "cBonusPickup" ||
			type == "cLegoTerrain" || type == "cSkyBox") {
			auto mesh_instance = memnew(MeshInstance3D);

			if (type == "cLegoTerrain")
				mesh_instance->set_layer_mask(RenderLayerTerrain);
			else if (type == "cSkyBox")
				mesh_instance->set_layer_mask(RenderLayerSkyBox);
			else
				mesh_instance->set_layer_mask(RenderLayerProps);

			add_child(mesh_instance);
			instances.insert(a.value().id, {mesh_instance});
		}
	}

	for (auto prop = change.propertyChanges.front(); prop; prop = prop.next()) {
		WRL::EntryID entry = prop.key().first;
		String type = wrl->get_entry_property(entry, "type");
		String prop_name = prop.key().second;
		if ((type == "cGeneralStatic" || type == "cGoldenBrick" || type == "cGeneralMobile" || type == "cBonusPickup" ||
			 type == "cSkyBox") &&
			prop_name == "model") {
			instances[entry.id].model_path = prop.value();
			pending.insert(entry.id);
		}

		else if (type == "cLegoTerrain" && (prop_name == "model" || prop_name == "texture_scale")) {
			Ref<TDF> tdf = memnew(TDF);
			tdf->load(custom_fs, wrl->get_entry_property(entry, "model"));
			Ref<TDFMesh> mesh = memnew(TDFMesh);
			mesh->set_texture_scale(wrl->get_entry_property(entry, "texture_scale"), assets);
			mesh->set_tdf(tdf, assets);
			instances[entry.id].mesh_instance->set_mesh(mesh);
		}

		else if (
			(type == "cGeneralStatic" || type == "cGoldenBrick" || type == "cGeneralMobile" ||
			 type == "cBonusPickup") &&
			(prop_name == "position" || prop_name == "rotation")) {
			instances[entry.id].mesh_instance->set_transform(
				Transform3D(
					Basis(wrl->get_entry_property(entry, "rotation")), wrl->get_entry_property(entry, "position"))
					.scaled({-1, 1, 1}));
		}

		else if (
			type == "cLegoTerrain" && (prop_name == "position" || prop_name == "rotation" || prop_name == "scale")) {
			instances[entry.id].mesh_instance->set_transform(
				Transform3D(
					Basis(wrl->get_entry_property(entry, "rotation")).scaled(wrl->get_entry_property(entry, "scale")),
					wrl->get_entry_property(entry, "position"))
					.scaled({-1, 1, 1}));
		}
	}
}

Viewer::Viewer(const CustomFS& p_custom_fs) : custom_fs(p_custom_fs), assets(custom_fs) {
	assets.add_loader(Ref(memnew(ImageAssetLoader)));
	assets.add_loader(Ref(memnew(ImageTextureAssetLoader)));
	assets.add_loader(Ref(memnew(IFLAssetLoader)));
	assets.add_loader(Ref(memnew(MDL2AssetLoader)));

	set_process(true);
	set_process_input(true);

	SubViewportContainer* viewport_container = memnew(SubViewportContainer);
	viewport_container->set_stretch(true);
	viewport_container->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	viewport_container->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	add_child(viewport_container);

	SubViewport* bg_viewport = memnew(SubViewport);
	bg_viewport->set_debug_draw(Viewport::DEBUG_DRAW_UNSHADED);
	viewport_container->add_child(bg_viewport);
	bg_camera = memnew(Camera3D);
	bg_camera->set_near(1);
	bg_camera->set_cull_mask(RenderLayerSkyBox);
	bg_viewport->add_child(bg_camera);

	SubViewport* viewport = memnew(SubViewport);
	viewport_container->add_child(viewport);
	camera = memnew(Camera3D);
	camera->set_near(1);
	camera->set_cull_mask(RenderLayerProps | RenderLayerTerrain);
	viewport->add_child(camera);

	MeshInstance3D* skybox_viewer = memnew(MeshInstance3D);
	skybox_viewer->set_mesh(memnew(QuadMesh));
	Ref<Shader> skybox_viewer_shader = memnew(Shader);
	skybox_viewer_shader->set_code(R"(
shader_type spatial;
render_mode unshaded;
uniform sampler2D skybox : hint_albedo;
void vertex() { POSITION = vec4(VERTEX.x * 2.0, VERTEX.y * -2.0, 1.0, 1.0); }
void fragment() { ALBEDO = texture(skybox, UV).rgb; }
)");
	Ref<ShaderMaterial> skybox_viewer_material = memnew(ShaderMaterial);
	skybox_viewer_material->set_shader(skybox_viewer_shader);
	skybox_viewer_material->set_shader_param("skybox", bg_viewport->get_texture());
	skybox_viewer->set_material_override(skybox_viewer_material);
	skybox_viewer->set_position(Vector3(0, 0, -2));
	camera->add_child(skybox_viewer);

	DirectionalLight3D* l = memnew(DirectionalLight3D);
	l->rotate_x(-Math_PI / 4);
	add_child(l);
}