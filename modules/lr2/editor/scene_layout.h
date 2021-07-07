#pragma once

#include "scene/gui/tree.h"
#include "wrl.h"

class SceneLayout : public Tree, public WRL::EventHandler {
	GDCLASS(SceneLayout, Tree);

  protected:
	void _notification(int p_what) {}
	virtual void _wrl_added(Ref<WRLEntry> entry, int index, bool synthetic) override;
	virtual void _wrl_removed(Ref<WRLEntry> entry, int index, bool synthetic) override;

  public:
	SceneLayout();
};