#pragma once

#include "scene/gui/box_container.h"
#include "scene/gui/scroll_container.h"
#include "wrl.h"

class Inspector : public ScrollContainer, public WRL::EventHandler {
	GDCLASS(Inspector, ScrollContainer)

	Ref<WRL::Entry> selected;

	VBoxContainer* vbox;

  protected:
	void _wrl_modified(Ref<WRL::Entry> entry, int index) override;
	void _wrl_selected(Ref<WRL::Entry> entry, int index) override;
	void _wrl_deselected(Ref<WRL::Entry> entry, int index) override;

  public:
	Inspector();
};