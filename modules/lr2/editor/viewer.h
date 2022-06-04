#pragma once

#include "../io/asset_manager.hpp"
#include "../io/custom_fs.hpp"
#include "../wrl/wrl.hpp"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/physics_body_3d.h"
#include "scene/gui/box_container.h"
#include "scene/main/viewport.h"

class Viewer : public BoxContainer, public WRL::EventHandler {
	GDCLASS(Viewer, BoxContainer);

  private:
	enum RenderLayer {
		RenderLayerProps = 1 << 0,
		RenderLayerTerrain = 1 << 1,
		RenderLayerSkyBox = 1 << 2,
	};

	Node3D* root;
	Camera3D* bg_camera;
	Camera3D* camera;
	enum class Mode { Default, FPS };
	Mode mode = Mode::Default;
	const float look_speed = 0.2 * (Math_PI / 180.0);
	const float move_speed = 100;

	struct Instance {
		Vector3 position;
		Quaternion rotation;
		Vector3 scale = {1, 1, 1};

		MeshInstance3D* mesh_instance;
		String model_path;

		CollisionObject3D* collider = nullptr;
	};
	struct Hasher {
		static _FORCE_INLINE_ uint32_t hash(const WRL::EntryID& key) { return HashMapHasherDefault::hash(key.id); }
	};
	HashMap<WRL::EntryID, Instance, Hasher> instances;

	HashSet<WRL::EntryID, Hasher> pending;

  protected:
	void input(const Ref<InputEvent>& p_event) override;
	void gui_input(const Ref<InputEvent>& p_event) override;
	void _notification(int p_what);

  private:
	const CustomFS& custom_fs;
	AssetManager assets;

  public:
	Viewer(const CustomFS&);

  protected:
	void _wrl_changed(const WRL::Change&, bool) override;
};