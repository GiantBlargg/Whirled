#include "viewer.h"

#include "core/os/keyboard.h"
#include "scene/3d/camera_3d.h"
#include "scene/3d/light_3d.h"
#include "scene/gui/subviewport_container.h"

void Viewer::input(const Ref<InputEvent>& p_event) {
	Ref<InputEventKey> k = p_event;
	if (right && k.is_valid())
		get_viewport()->set_input_as_handled();
}

void Viewer::gui_input(const Ref<InputEvent>& p_event) {
	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid()) {
		if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_RIGHT) {
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

			if (i->is_key_pressed(Key::KEY_W))
				movement.z--;
			if (i->is_key_pressed(Key::KEY_S))
				movement.z++;
			if (i->is_key_pressed(Key::KEY_A))
				movement.x--;
			if (i->is_key_pressed(Key::KEY_D))
				movement.x++;
			if (i->is_key_pressed(Key::KEY_Q))
				movement.y--;
			if (i->is_key_pressed(Key::KEY_E))
				movement.y++;

			if (i->is_key_pressed(Key::KEY_SHIFT))
				movement *= 5;
			if (i->is_key_pressed(Key::KEY_CTRL))
				movement /= 5;

			movement *= get_process_delta_time() * move_speed;
			camera->translate(movement);
		}

		for (auto p = pending.front(); p; p = p->next()) {
			String name = p->get();
			Instance i = instances[name];
			String resolved_path = "res://" + i.model_path;
			switch (ResourceLoader::load_threaded_get_status(resolved_path)) {
			case ResourceLoader::THREAD_LOAD_INVALID_RESOURCE:
				ResourceLoader::load_threaded_request(resolved_path, "Mesh", true);
				break;
			case ResourceLoader::THREAD_LOAD_IN_PROGRESS:
				break;
			case ResourceLoader::THREAD_LOAD_FAILED:
				Error err;
				ResourceLoader::load_threaded_get(resolved_path, &err);
				ERR_PRINT("Failed to load: " + resolved_path);
				break;
			case ResourceLoader::THREAD_LOAD_LOADED:
				i.mesh_instance->set_mesh(ResourceLoader::load_threaded_get(resolved_path));
				pending.erase(p);
				break;
			}
		}
	}
}

void Viewer::_wrl_added(Ref<WRLEntry> entry, int index) {
	Ref<WRLGeneralStatic> gs = entry;
	if (gs.is_valid()) {
		auto mesh_instance = memnew(MeshInstance3D);
		mesh_instance->set_layer_mask(RenderLayerProps);
		add_child(mesh_instance);
		instances.insert(entry->name, {mesh_instance});
	}

	Ref<WRLSkyBox> sb = entry;
	if (sb.is_valid()) {
		auto mesh_instance = memnew(MeshInstance3D);
		mesh_instance->set_layer_mask(RenderLayerSkyBox);
		add_child(mesh_instance);
		instances.insert(entry->name, {mesh_instance});
	}

	_wrl_modified(entry, index);
}

void Viewer::_wrl_modified(Ref<WRLEntry> entry, int index) {
	Ref<WRLGeneralStatic> gs = entry;
	if (gs.is_valid()) {
		Instance& i = instances[entry->name];
		i.mesh_instance->set_transform(Transform3D(Basis(gs->rotation), gs->position).scaled({-1, 1, 1}));
		if (i.model_path != gs->model) {
			i.model_path = gs->model;
			pending.insert(entry->name);
		}
	}

	Ref<WRLSkyBox> sb = entry;
	if (sb.is_valid()) {
		Instance& i = instances[entry->name];
		if (i.model_path != sb->model) {
			i.model_path = sb->model;
			pending.insert(entry->name);
		}
	}
}

void Viewer::_wrl_removed(Ref<WRLEntry> entry, int index) {
	if (instances.has(entry->name)) {
		Instance& i = instances[entry->name];
		i.mesh_instance->queue_delete();
		instances.erase(entry->name);
	}
}

Viewer::Viewer() {
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
	bg_camera->set_cull_mask(RenderLayerSkyBox);
	bg_viewport->add_child(bg_camera);

	SubViewport* viewport = memnew(SubViewport);
	viewport->set_transparent_background(true);
	viewport_container->add_child(viewport);
	camera = memnew(Camera3D);
	camera->set_cull_mask(RenderLayerProps);
	viewport->add_child(camera);

	DirectionalLight3D* l = memnew(DirectionalLight3D);
	l->rotate_x(-Math_PI / 4);
	add_child(l);
}