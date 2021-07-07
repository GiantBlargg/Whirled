#include "scene_layout.h"

void SceneLayout::_wrl_added(Ref<WRLEntry> entry, int index,bool synthetic) {
	TreeItem* item = create_item(nullptr, index);
	item->set_text(0, entry->name);
}

void SceneLayout::_wrl_removed(Ref<WRLEntry> entry, int index,bool synthetic) {
	TreeItem* root = get_root();
	root->remove_child(root->get_child(index));
}

SceneLayout::SceneLayout() {
	create_item();
	set_hide_root(true);
}