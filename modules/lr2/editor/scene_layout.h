#pragma once

#include "../wrl/wrl.hpp"
#include "scene/gui/tree.h"

class SceneLayout : public Tree, public WRL::EventHandler {
	GDCLASS(SceneLayout, Tree);

  private:
	void _item_selected();

  protected:
	void _notification(int p_what);

	virtual void _wrl_event(const WRL::Event&) override;
};