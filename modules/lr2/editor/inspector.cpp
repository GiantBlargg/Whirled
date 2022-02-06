#include "inspector.h"

#include "scene/gui/label.h"

void Inspector::_wrl_event(const WRL::Event& event) {
	switch (event.event_type) {
	case WRL::Event::Type::Modified:
		update_widgets(event);
		break;
	case WRL::Event::Type::Selected:
		if (selected.id != -1)
			_wrl_event(WRL::Event{
				.event_type = WRL::Event::Type::Deselected,
				.entry_type = wrl->get_Entry_EntryType(selected),
				.id = selected,
				// .index = wrl->get_index(selected),
			});

		selected = event.id;

		vbox = memnew(VBoxContainer);
		add_child(vbox);
		vbox->set_h_size_flags(SIZE_EXPAND_FILL);
		init_widgets(wrl->get_Entry_EntryType(selected));
		_wrl_emit_modified(event);
		break;
	case WRL::Event::Type::Deselected:
		selected.id = -1;
		vbox->queue_delete();
		break;
	}
}
