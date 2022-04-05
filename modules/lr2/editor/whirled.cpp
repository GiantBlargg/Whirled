#include "whirled.h"

#include "inspector.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/split_container.h"

void Whirled::_menu_new() {
	file_path = "";
	wrl->clear();
}
void Whirled::_menu_open() {
	_file_reset();
	file->set_file_mode(CustomFileDialog::FILE_MODE_OPEN_FILE);
	file->connect("file_selected", callable_mp(this, &Whirled::_file_open));
	file->popup_centered_clamped(Size2(600, 400));
}
void Whirled::_menu_save() {
	if (file_path == "") {
		_menu_save_as();
	} else {
		_file_save_as(file_path);
	}
}
void Whirled::_menu_save_as() {
	_file_reset();
	file->set_file_mode(CustomFileDialog::FILE_MODE_SAVE_FILE);
	file->connect("file_selected", callable_mp(this, &Whirled::_file_save_as));
	file->popup_centered_clamped(Size2(600, 400));
}

void Whirled::_file_open(String path) {
	file_path = path;
	FileAccessRef file = custom_fs.FileAccess_open(path, FileAccess::ModeFlags::READ);
	wrl->load(file);
	file->close();
}
void Whirled::_file_save_as(String path) {
	file_path = path;
	FileAccessRef file = custom_fs.FileAccess_open(path, FileAccess::ModeFlags::WRITE);
	// wrl->save(file);
	file->close();
}
void Whirled::_file_reset() {
	if (file->is_connected("file_selected", callable_mp(this, &Whirled::_file_open))) {
		file->disconnect("file_selected", callable_mp(this, &Whirled::_file_open));
	}
	if (file->is_connected("file_selected", callable_mp(this, &Whirled::_file_save_as))) {
		file->disconnect("file_selected", callable_mp(this, &Whirled::_file_save_as));
	}
}

Whirled::Whirled(const CustomFS p_custom_fs) : custom_fs(p_custom_fs) {
	wrl.instantiate();

	// TODO: Theme

	PanelContainer* base = memnew(PanelContainer);
	add_child(base);
	base->set_anchors_and_offsets_preset(Control::PRESET_WIDE);

	file = memnew(CustomFileDialog);
	file->set_custom_fs(custom_fs);
	file->set_current_dir("/game data/SAVED WORLDS");
	file->add_filter("*.WRL; LR2 Worlds");
	base->add_child(file);

	VBoxContainer* main_vbox = memnew(VBoxContainer);
	base->add_child(main_vbox);

	HBoxContainer* menu = memnew(HBoxContainer);
	main_vbox->add_child(menu);

	Button* new_button = memnew(Button);
	menu->add_child(new_button);
	new_button->set_text("New");
	new_button->connect("pressed", callable_mp(this, &Whirled::_menu_new));

	Button* open_button = memnew(Button);
	menu->add_child(open_button);
	open_button->set_text("Open");
	open_button->connect("pressed", callable_mp(this, &Whirled::_menu_open));

	Button* save_button = memnew(Button);
	menu->add_child(save_button);
	save_button->set_text("Save");
	save_button->set_disabled(true);
	save_button->connect("pressed", callable_mp(this, &Whirled::_menu_save));

	Button* save_as_button = memnew(Button);
	menu->add_child(save_as_button);
	save_as_button->set_text("Save As");
	save_as_button->set_disabled(true);
	save_as_button->connect("pressed", callable_mp(this, &Whirled::_menu_save_as));

	HSplitContainer* right_drawer_split = memnew(HSplitContainer);
	main_vbox->add_child(right_drawer_split);
	right_drawer_split->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	right_drawer_split->set_split_offset(210);

	scene = memnew(SceneLayout);
	right_drawer_split->add_child(scene);
	scene->wrl_connect(wrl);

	HSplitContainer* left_drawer_split = memnew(HSplitContainer);
	right_drawer_split->add_child(left_drawer_split);

	viewer = memnew(Viewer(custom_fs));
	left_drawer_split->add_child(viewer);
	left_drawer_split->set_split_offset(-210);
	viewer->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	viewer->wrl_connect(wrl);

	Inspector* inspector = memnew(Inspector);
	left_drawer_split->add_child(inspector);
	inspector->wrl_connect(wrl);
}