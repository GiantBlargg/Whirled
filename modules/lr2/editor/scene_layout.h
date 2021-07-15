#pragma once

#include "scene/gui/tree.h"
#include "wrl.h"

class SceneLayout : public Tree, public WRL::EventHandler {
	GDCLASS(SceneLayout, Tree);

  private:
	Ref<WRL> wrl;

	void _item_selected();

  protected:
	void _notification(int p_what);
	virtual void _wrl_added(Ref<WRLEntry> entry, int index) override;
	virtual void _wrl_removed(Ref<WRLEntry> entry, int index) override;
	virtual void _wrl_selected(Ref<WRLEntry> entry, int index) override;
	virtual void _wrl_deselected(Ref<WRLEntry> entry, int index) override;

  public:
	SceneLayout(Ref<WRL> wrl);
};