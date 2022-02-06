#include "scene_layout.h"

void SceneLayout::_item_selected() {
	TreeItem* item = get_selected();
	String name = item->get_text(0);
	int index = wrl->get_index(name);
	wrl->select(index);
}

void SceneLayout::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		connect("item_selected", callable_mp(this, &SceneLayout::_item_selected));
		create_item();
		set_hide_root(true);
	}
}

void SceneLayout::_wrl_event(const WRL::Event& event) {
	switch (event.event_type) {
	case WRL::Event::Type::Inited:
		_wrl_emit_add_all();
		return;

	case WRL::Event::Type::Cleared:
		_wrl_emit_remove_all();
		return;

	case WRL::Event::Type::Added: {
		TreeItem* item = create_item(nullptr, event.index);
		item->set_text(0, wrl->get_Entry_name(event.id));
	}
		return;

	case WRL::Event::Type::Removed: {
		TreeItem* root = get_root();
		root->remove_child(root->get_child(event.index));
	}
		return;

	case WRL::Event::Type::Selected:
		get_root()->get_child(event.index)->select(0);
		return;

	case WRL::Event::Type::Deselected:
		get_root()->get_child(event.index)->deselect(0);
		return;
	}
}
