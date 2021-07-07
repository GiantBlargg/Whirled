#pragma once

#include "scene/3d/mesh_instance_3d.h"
#include "scene/gui/box_container.h"
#include "scene/main/viewport.h"
#include "wrl.h"

class Viewer : public BoxContainer, public WRL::EventHandler {
	GDCLASS(Viewer, BoxContainer);

  private:
	SubViewport* viewport;

	bool right = false;
	Camera3D* camera;
	const float look_speed = 0.2 * (Math_PI / 180.0);
	float move_speed = 100;

	struct Instance {
		MeshInstance3D* mesh_instance;
		String model_path;
	};
	Map<String, Instance> instances;

	Set<String> pending;

  protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("_input"), &Viewer::_input);
		ClassDB::bind_method(D_METHOD("_gui_input"), &Viewer::_gui_input);
	}
	void _input(Ref<InputEvent> p_event);
	void _gui_input(Ref<InputEvent> p_event);
	void _notification(int p_what);

  public:
	Viewer();

  protected:
	void _wrl_added(Ref<WRLEntry> entry, int index, bool synthetic) override;
	void _wrl_modified(Ref<WRLEntry> entry, int index, bool synthetic) override;
	void _wrl_removed(Ref<WRLEntry> entry, int index, bool synthetic) override;
};