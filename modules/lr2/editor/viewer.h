#pragma once

#include "scene/gui/box_container.h"

class Viewer : public BoxContainer {
	GDCLASS(Viewer, BoxContainer);

  private:
	bool right = false;
	Camera3D* camera;
	const float look_speed = 0.2 * (Math_PI / 180.0);
	float move_speed = 100;

  protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("_input", "event"), &Viewer::_input);
		ClassDB::bind_method(D_METHOD("_gui_input", "event"), &Viewer::_gui_input);
	}
	void _input(Ref<InputEvent> p_event);
	void _gui_input(Ref<InputEvent> p_event);
	void _notification(int p_what);

  public:
	Viewer();
};