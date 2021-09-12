#pragma once

#include "scene/3d/mesh_instance_3d.h"
#include "scene/gui/box_container.h"
#include "scene/main/viewport.h"
#include "wrl.h"

class Viewer : public BoxContainer, public WRL::EventHandler {
	GDCLASS(Viewer, BoxContainer);

  private:
	enum RenderLayer {
		RenderLayerProps = 1 << 0,
		RenderLayerTerrain = 1 << 1,
		RenderLayerSkyBox = 1 << 2,
	};

	Camera3D* bg_camera;
	Camera3D* camera;
	bool right = false;
	const float look_speed = 0.2 * (Math_PI / 180.0);
	float move_speed = 100;

	struct Instance {
		MeshInstance3D* mesh_instance;
		String model_path;
	};
	Map<String, Instance> instances;

	Set<String> pending;

  protected:
	void input(const Ref<InputEvent>& p_event) override;
	void gui_input(const Ref<InputEvent>& p_event) override;
	void _notification(int p_what);

  public:
	Viewer();

  protected:
	void _wrl_added(Ref<WRL::Entry> entry, int index) override;
	void _wrl_modified(Ref<WRL::Entry> entry, int index) override;
	void _wrl_removed(Ref<WRL::Entry> entry, int index) override;
};