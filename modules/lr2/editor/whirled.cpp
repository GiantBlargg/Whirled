#include "whirled.h"

#include "core/string/translation.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/split_container.h"
#include "servers/navigation_server_3d.h"

Whirled::Whirled() {
	Input::get_singleton()->set_use_accumulated_input(true);

	NavigationServer3D::get_singleton()->set_active(false);
	PhysicsServer3D::get_singleton()->set_active(false);
	PhysicsServer2D::get_singleton()->set_active(false);
	ScriptServer::set_scripting_enabled(false);
	TranslationServer::get_singleton()->set_enabled(false);

	DisplayServer::get_singleton()->window_set_min_size(Size2(1024, 600));

	// TODO: Theme

	PanelContainer* base = memnew(PanelContainer);
	add_child(base);
	base->set_anchors_and_offsets_preset(Control::PRESET_WIDE);

	VBoxContainer* main_vbox = memnew(VBoxContainer);
	base->add_child(main_vbox);

	HBoxContainer* menu = memnew(HBoxContainer);
	main_vbox->add_child(menu);

	Button* new_button = memnew(Button);
	menu->add_child(new_button);
	new_button->set_text("New");

	Button* open_button = memnew(Button);
	menu->add_child(open_button);
	open_button->set_text("Open");

	HSplitContainer* right_drawer_split = memnew(HSplitContainer);
	main_vbox->add_child(right_drawer_split);
	right_drawer_split->set_v_size_flags(Control::SIZE_EXPAND_FILL);

	ScrollContainer* right_drawer = memnew(ScrollContainer);
	right_drawer_split->add_child(right_drawer);

	HSplitContainer* left_drawer_split = memnew(HSplitContainer);
	right_drawer_split->add_child(left_drawer_split);

	Control* main_viewport = memnew(Control);
	left_drawer_split->add_child(main_viewport);
	main_viewport->set_h_size_flags(Control::SIZE_EXPAND_FILL);

	ScrollContainer* left_drawer = memnew(ScrollContainer);
	left_drawer_split->add_child(left_drawer);
}