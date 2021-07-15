#include "scene_layout.h"

void SceneLayout::_item_selected() {
	TreeItem* item = get_selected();
	String name = item->get_text(0);
	wrl->select(name);
}

void SceneLayout::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		connect("item_selected", callable_mp(this, &SceneLayout::_item_selected));
		create_item();
		set_hide_root(true);
	}
}

void SceneLayout::_wrl_added(Ref<WRLEntry> entry, int index) {
	TreeItem* item = create_item(nullptr, index);
	item->set_text(0, entry->name);
}

void SceneLayout::_wrl_removed(Ref<WRLEntry> entry, int index) {
	TreeItem* root = get_root();
	root->remove_child(root->get_child(index));
}

void SceneLayout::_wrl_selected(Ref<WRLEntry> entry, int index) { get_root()->get_child(index)->select(0); };
void SceneLayout::_wrl_deselected(Ref<WRLEntry> entry, int index) {
	get_root()->get_child(index)->deselect(0);
	ensure_cursor_is_visible();
};

SceneLayout::SceneLayout(Ref<WRL> w) : wrl(w) { wrl->add_event_handler(this); }