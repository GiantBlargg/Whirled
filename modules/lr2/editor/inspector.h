#pragma once

#include "scene/gui/box_container.h"
#include "scene/gui/scroll_container.h"
#include "wrl.h"

class Inspector : public ScrollContainer, public WRL::EventHandler {
	GDCLASS(Inspector, ScrollContainer)

	Ref<WRLEntry> selected;

	VBoxContainer* vbox;

  protected:
	virtual void _wrl_modified(Ref<WRLEntry> entry, int index) override;
	virtual void _wrl_selected(Ref<WRLEntry> entry, int index) override;
	virtual void _wrl_deselected(Ref<WRLEntry> entry, int index) override;

  public:
	Inspector();
};