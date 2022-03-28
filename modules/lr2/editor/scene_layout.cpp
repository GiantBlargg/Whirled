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

void SceneLayout::_wrl_changed(const WRL::Change& change, bool reset) {
	TreeItem* root = get_root();
	for (auto r = change.removed.back(); r; r = r.prev()) {
		root->remove_child(root->get_child(r.key()));
	}
	for (auto a = change.added.front(); a; a = a.next()) {
		TreeItem* item = create_item(nullptr, a.key());
		item->set_text(0, wrl->get_entry_property(a.value(), "name"));
	}
	if (change.select_changed) {
		if (change.select.first == -1) {
			if (get_selected())
				get_selected()->deselect(0);
		} else {
			root->get_child(change.select.first)->select(0);
		}
	}
}
