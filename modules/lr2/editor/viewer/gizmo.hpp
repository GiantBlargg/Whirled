#pragma once

#include "core/math/vector2.h"
#include "core/templates/vector.h"
#include "scene/3d/mesh_instance_3d.h"

#include "lr2/wrl/wrl.hpp"

enum GizmoDir { X_AXIS, Y_AXIS, Z_AXIS, OTHER, ACTIVE };

class CollisionShape3D;

class Gizmo : public MeshInstance3D, public WRL::EventHandler {
	GDCLASS(Gizmo, MeshInstance3D);

  private:
	Ref<Material> inactive_mat;
	Ref<Material> active_mat;
	Transform3D camera_pos;

	CollisionShape3D* cshape;

  protected:
	WRL::EntryID entry;
	String position;

	void set_shape(const Ref<Shape3D>& shape, const Transform3D& transform = Transform3D());

	Vector2 plane_position(const Vector3& mouse_origin, const Vector3& mouse_normal);

  public:
	void mouse_over(bool);
	virtual void interact(const Vector3& mouse_origin, const Vector3& mouse_normal, bool reset = false) = 0;
	void update_camera(const Transform3D& transform) {
		camera_pos = transform;
		update_scale();
	}

  protected:
	bool lite_init() override { return true; }
	void _wrl_changed(const WRL::Change&, bool reset) override;

  private:
	void update_scale();

  protected:
	Gizmo(WRL::EntryID, String position, GizmoDir);
};

class Trans1DGizmo : public Gizmo {
	GDCLASS(Trans1DGizmo, Gizmo)

	constexpr static real_t base_offset = 0, head_offset = 0.7, tip_offset = 1, base_radius = 0.03, head_radius = 0.1;
	constexpr static int rot_segments = 12;

	real_t pos_offset;

  public:
	void interact(const Vector3& mouse_origin, const Vector3& mouse_normal, bool reset) override;

	Trans1DGizmo(WRL::EntryID, String position, GizmoDir);
};

class RotateGizmo : public Gizmo {
	GDCLASS(RotateGizmo, Gizmo)

	constexpr static real_t large_radius = 0.5, small_radius = 0.03;
	constexpr static int rot_segments = 64;

	String rotation;

	real_t last_angle;

  public:
	void interact(const Vector3& mouse_origin, const Vector3& mouse_normal, bool reset) override;

	RotateGizmo(WRL::EntryID, String position, String rotation, GizmoDir);
};