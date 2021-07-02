#include "viewer.h"

#include "core/os/keyboard.h"
#include "scene/3d/camera_3d.h"
#include "scene/3d/light_3d.h"
#include "scene/gui/subviewport_container.h"

void Viewer::_input(Ref<InputEvent> p_event) {
	Ref<InputEventKey> k = p_event;
	if (right && k.is_valid())
		get_viewport()->set_input_as_handled();
}

void Viewer::_gui_input(Ref<InputEvent> p_event) {
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
			camera->set_rotation(Vector3(CLAMP(old_rot.x - rel.y, -Math_PI / 2, Math_PI / 2), (old_rot.y - rel.x), 0));
		}
	}
}

void Viewer::_notification(int p_what) {
	if (p_what == NOTIFICATION_PROCESS) {
		if (right) {
			Vector3 movement;

			auto i = Input::get_singleton();

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

void Viewer::_wrl_event(WRL::WRLEvent event_type, String name, Ref<WRLEntry> entry) {
	switch (event_type) {
	case WRL::Added:
		if (entry->type == "cGeneralStatic") {
			auto mesh_instance = memnew(MeshInstance3D);
			viewport->add_child(mesh_instance);
			instances.insert(name, {mesh_instance});
			_wrl_event(WRL::Modifed, name, entry);
		}
		break;
	case WRL::WRLEvent::Modifed:
		if (entry->type == "cGeneralStatic") {
			Ref<WRLGeneralStatic> gs = entry;
			Instance& i = instances[name];
			i.mesh_instance->set_transform(Transform3D(Basis(gs->rotation), gs->position));
			if (i.model_path != gs->model) {
				i.model_path = gs->model;
				pending.insert(name);
			}
		}
		break;
	case WRL::WRLEvent::Removed:
		if (instances.has(name)) {
			Instance& i = instances[name];
			i.mesh_instance->queue_delete();
			instances.erase(name);
		}
		break;
	case WRL::WRLEvent::Renamed:
		break;
	}
}

Viewer::Viewer() {
	set_process(true);

	SubViewportContainer* viewport_container = memnew(SubViewportContainer);
	viewport_container->set_stretch(true);
	viewport_container->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	viewport_container->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	add_child(viewport_container);

	viewport = memnew(SubViewport);
	viewport_container->add_child(viewport);

	DirectionalLight3D* l = memnew(DirectionalLight3D);
	l->rotate_x(Math_PI / 4);
	viewport->add_child(l);

	camera = memnew(Camera3D);
	camera->translate(Vector3(0, 0, 5));
	camera->set_current(true);

	viewport->add_child(camera);
}