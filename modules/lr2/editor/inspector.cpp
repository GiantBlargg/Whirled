#include "inspector.h"

#include "scene/gui/label.h"

void Inspector::_wrl_modified(Ref<WRLEntry> entry, int index) {
	if (entry != selected)
		return;
}

void Inspector::_wrl_selected(Ref<WRLEntry> entry, int index) {
	selected = entry;

	Label* name_label = memnew(Label);
	vbox->add_child(name_label);
	name_label->set_text(entry->name);

	Label* type_label = memnew(Label);
	vbox->add_child(type_label);
	type_label->set_text(entry->type);

	Label* not_imp_label = memnew(Label);
	vbox->add_child(not_imp_label);
	not_imp_label->set_text("This type does not have an implemented inspector yet.");

	_wrl_modified(entry, index);
}

void Inspector::_wrl_deselected(Ref<WRLEntry> entry, int index) {
	selected.unref();
	vbox->queue_delete();
	vbox = memnew(VBoxContainer);
	add_child(vbox);
}

Inspector::Inspector() {
	vbox = memnew(VBoxContainer);
	add_child(vbox);
}
