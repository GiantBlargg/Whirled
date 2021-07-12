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

			if (i->is_key_pressed(Key::KEY_SHIFT))
				movement *= 5;
			if (i->is_key_pressed(Key::KEY_CTRL))
				movement /= 5;

			movement *= get_process_delta_time() * move_speed;
			camera->translate(movement);
		}

#define BATCH 1

#if BATCH
		ResourceLoader::load_threaded_request_batch(request_files, "Mesh", true);
#else
		for (int i = 0; i < request_files.size(); i++) {
			ResourceLoader::load_threaded_request(request_files[i], "Mesh", true);
		}
#endif

#if BATCH
		Vector<ResourceLoader::ThreadLoadStatus> status = ResourceLoader::load_threaded_get_status_batch(pending_files);
#else
		Vector<ResourceLoader::ThreadLoadStatus> status;
		status.resize(pending_files.size());
		for (int s = 0; s < status.size(); s++) {
			status.set(s, ResourceLoader::load_threaded_get_status(pending_files[s]));
		}
#endif

#if BATCH
		Vector<String> done;
		int wtf = 0;

		for (int s = 0; s < status.size(); s++) {
			switch (status[s]) {
			case ResourceLoader::THREAD_LOAD_INVALID_RESOURCE:
				request_files.append(pending_files[s]);
				break;
			case ResourceLoader::THREAD_LOAD_IN_PROGRESS:
				break;
			case ResourceLoader::THREAD_LOAD_FAILED:
				Error err;
				ResourceLoader::load_threaded_get(pending_files[s], &err);
				ERR_PRINT("Failed to load: " + pending_files[s]);
				break;
			case ResourceLoader::THREAD_LOAD_LOADED: {
				done.append(pending_files[s]);
				wtf++;
			} break;
			}
		}
		Vector<RES> res = ResourceLoader::load_threaded_get_batch(done);
		for (int s = 0, p = 0, r = 0; s < status.size(); s++, p++) {
			if (status[s] != ResourceLoader::THREAD_LOAD_LOADED)
				continue;

			Instance i = instances[pending_names[p]];
			if (i.model_path == pending_files[p])
				i.mesh_instance->set_mesh(res[r]);
			r++;
			pending_names.remove(p);
			pending_files.remove(p);
			p--;
		}
#else
		for (int s = 0, p = 0; s < status.size(); s++, p++) {
			switch (status[s]) {
			case ResourceLoader::THREAD_LOAD_INVALID_RESOURCE:
				request_files.append(pending_files[p]);
				break;
			case ResourceLoader::THREAD_LOAD_IN_PROGRESS:
				break;
			case ResourceLoader::THREAD_LOAD_FAILED:
				Error err;
				ResourceLoader::load_threaded_get(pending_files[p], &err);
				ERR_PRINT("Failed to load: " + pending_files[p]);
				break;
			case ResourceLoader::THREAD_LOAD_LOADED: {
				Instance i = instances[pending_names[p]];
				if (i.model_path == pending_files[p])
					i.mesh_instance->set_mesh(ResourceLoader::load_threaded_get(pending_files[p]));
				pending_names.remove(p);
				pending_files.remove(p);
				p--;
			} break;
			}
		}
#endif
	}
}

void Viewer::_wrl_added(Ref<WRLEntry> entry, int index, bool synthetic) {
	Ref<WRLGeneralStatic> gs = entry;
	if (gs.is_valid()) {
		auto mesh_instance = memnew(MeshInstance3D);
		viewport->add_child(mesh_instance);
		instances.insert(entry->name, {mesh_instance});
	}
}

void Viewer::_wrl_modified(Ref<WRLEntry> entry, int index, bool synthetic) {
	Ref<WRLGeneralStatic> gs = entry;
	if (gs.is_valid()) {
		Instance& i = instances[entry->name];
		i.mesh_instance->set_transform(Transform3D(Basis(gs->rotation), gs->position).scaled({-1, 1, 1}));

		String model_path = "res://" + gs->model;
		if (i.model_path != model_path) {
			i.model_path = model_path;
			pending_names.append(entry->name);
			pending_files.append(model_path);
			request_files.append(model_path);
		}
	}
}

void Viewer::_wrl_removed(Ref<WRLEntry> entry, int index, bool synthetic) {
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

	viewport = memnew(SubViewport);
	viewport_container->add_child(viewport);

	DirectionalLight3D* l = memnew(DirectionalLight3D);
	l->rotate_x(-Math_PI / 4);
	viewport->add_child(l);

	camera = memnew(Camera3D);
	camera->set_current(true);

	viewport->add_child(camera);
}