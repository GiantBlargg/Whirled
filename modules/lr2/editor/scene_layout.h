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
	void _wrl_added(Ref<WRL::Entry> entry, int index) override;
	void _wrl_removed(Ref<WRL::Entry> entry, int index) override;
	void _wrl_selected(Ref<WRL::Entry> entry, int index) override;
	void _wrl_deselected(Ref<WRL::Entry> entry, int index) override;

  public:
	SceneLayout(Ref<WRL> wrl);
};