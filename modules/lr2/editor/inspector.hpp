#pragma once

#include "scene/gui/box_container.h"
#include "scene/gui/scroll_container.h"

#include "lr2/wrl/wrl.hpp"

class Widget;

class Inspector : public ScrollContainer, public WRL::EventHandler {
	GDCLASS(Inspector, ScrollContainer)

  private:
	WRL::EntryID selected;

	VBoxContainer* vbox = nullptr;

  protected:
	void _wrl_changed(const WRL::Change&, bool) override;

  public:
	Inspector() { set_horizontal_scroll_mode(ScrollContainer::SCROLL_MODE_DISABLED); }
};