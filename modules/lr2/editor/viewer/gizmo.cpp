#include "gizmo.hpp"

#include "scene/3d/collision_shape_3d.h"
#include "scene/3d/physics_body_3d.h"
#include "scene/main/viewport.h"
#include "scene/resources/capsule_shape_3d.h"
#include "scene/resources/primitive_meshes.h"
#include "scene/resources/surface_tool.h"

#include "layer.hpp"

Ref<Mesh> arrow_mesh;
Ref<Mesh> ring_mesh;
Ref<Shape3D> ring_shape;

Vector<Ref<Material>> gizmo_mats;
Color gizmo_colours[] = {
	{0.96, 0.20, 0.32, 0.5}, {0.53, 0.84, 0.01, 0.5}, {0.16, 0.55, 0.96, 0.5}, {0.55, 0.55, 0.55, 0.5},
	{0.96, 0.20, 0.32, 0.9}, {0.53, 0.84, 0.01, 0.9}, {0.16, 0.55, 0.96, 0.9}, {0.55, 0.55, 0.55, 0.9},
};

void unref_gizmo() {
	arrow_mesh.unref();
	ring_mesh.unref();
	ring_shape.unref();
	gizmo_mats.clear();
}

Ref<Mesh> build_rotation_solid(Vector<Vector2> profile, int rot_segments) {

	Vector<Vector3> pos;
	Vector<Vector3> normal;
	Vector<int> index;

	float rot_per_seg = 2 * M_PI / static_cast<float>(rot_segments);

	for (int i = 0; i < profile.size() - 1; i++) {
		const Vector2 bot2 = profile[i];
		const Vector3 bot3(bot2.x, bot2.y, 0);
		const Vector2 top2 = profile[i + 1];
		const Vector3 top3(top2.x, top2.y, 0);
		const Vector2 dir = bot2 - top2;
		const Vector2 norm2 = Vector2(dir.y, -dir.x).normalized();
		const Vector3 norm3(norm2.x, norm2.y, 0);
		const int ring_offset = i * 2 * rot_segments;

		for (int j = 0; j < rot_segments; j++) {
			float rot = rot_per_seg * j;
			pos.append(bot3.rotated({1, 0, 0}, rot));
			pos.append(top3.rotated({1, 0, 0}, rot));
			if (bot2.y != 0)
				normal.append(norm3.rotated({1, 0, 0}, rot));
			else
				normal.append({0, 0, 0});
			if (top2.y != 0)
				normal.append(norm3.rotated({1, 0, 0}, rot));
			else
				normal.append({0, 0, 0});

			const int bl = ring_offset + (j * 2);
			const int tl = ring_offset + (j * 2) + 1;
			const int br = ring_offset + (((j + 1) % rot_segments) * 2);
			const int tr = ring_offset + (((j + 1) % rot_segments) * 2) + 1;
			if (bot2.y != 0 && top2.y != 0)
				index.append_array({bl, tl, br, br, tl, tr});
			else if (bot2.y != 0 && top2.y == 0)
				index.append_array({bl, tl, br});
			else if (bot2.y == 0 && top2.y != 0)
				index.append_array({br, tl, tr});
		}
	}
	Array surface_arrays;
	surface_arrays.resize(Mesh::ArrayType::ARRAY_MAX);
	surface_arrays.set(Mesh::ArrayType::ARRAY_VERTEX, pos);
	surface_arrays.set(Mesh::ArrayType::ARRAY_NORMAL, normal);
	surface_arrays.set(Mesh::ArrayType::ARRAY_INDEX, index);

	Ref<ArrayMesh> mesh;
	mesh.instantiate();
	mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, surface_arrays);

	return mesh;
}

void Gizmo::set_shape(const Ref<Shape3D>& shape, const Transform3D& transform) {
	cshape->set_transform(transform);
	cshape->set_shape(shape);
}
Vector2 Gizmo::plane_position(const Vector3& mouse_origin, const Vector3& mouse_normal) {
	Vector3 planeOrigin = get_global_transform().origin;

	Vector3 globalTangent = get_global_transform().basis.get_column(1).normalized();
	Vector3 globalBitangent = get_global_transform().basis.get_column(2).normalized();

	Vector3 originOffset = camera_pos.origin - planeOrigin;

	real_t M = Basis(globalTangent, globalBitangent, mouse_normal).determinant();
	real_t u = Basis(originOffset, globalBitangent, mouse_normal).determinant() / M;
	real_t v = Basis(mouse_normal, globalTangent, originOffset).determinant() / M;

	return Vector2(u, v);
}

void Gizmo::mouse_over(bool over) { set_material_override(over ? active_mat : inactive_mat); }
void Gizmo::_wrl_changed(const WRL::Change& change, bool reset) {
	if (reset) {
		set_position(wrl->get_entry_property(entry, position));
	} else {
		if (change.propertyChanges.has({entry, position})) {
			set_position(change.propertyChanges.get({entry, position}));
		}
	}

	update_scale();
}

void Gizmo::update_scale() {
	real_t distance = camera_pos.origin.distance_to(get_global_transform().origin);
	Vector3 one_vector(1, 1, 1);
	set_scale(one_vector * 0.15 * distance);
}

Gizmo::Gizmo(WRL::EntryID p_entry, String p_position, GizmoDir gd) : entry(p_entry), position(p_position) {
	set_layer_mask(LayerGizmo);

	if (gizmo_mats.is_empty()) {
		for (auto const& c : gizmo_colours) {
			Ref<StandardMaterial3D> mat = memnew(StandardMaterial3D);
			mat->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
			mat->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);
			mat->set_render_priority(StandardMaterial3D::RENDER_PRIORITY_MAX);
			mat->set_flag(StandardMaterial3D::FLAG_DISABLE_DEPTH_TEST, true);
			mat->set_albedo(c);
			gizmo_mats.append(mat);
		}
	}

	inactive_mat = gizmo_mats[gd];
	active_mat = gizmo_mats[gd | GizmoDir::ACTIVE];

	StaticBody3D* collider = memnew(StaticBody3D);
	collider->set_collision_layer(get_layer_mask());
	add_child(collider);
	cshape = memnew(CollisionShape3D);
	collider->add_child(cshape, true);

	mouse_over(false);
}

Trans1DGizmo::Trans1DGizmo(WRL::EntryID entry, String position, GizmoDir gd) : Gizmo(entry, position, gd) {
	if (arrow_mesh.is_null()) {
		const static Vector<Vector2> arrow_profile = {
			{base_offset, 0},
			{base_offset, base_radius},
			{head_offset, base_radius},
			{head_offset, head_radius},
			{tip_offset, 0}};
		arrow_mesh = build_rotation_solid(arrow_profile, rot_segments);
	}

	set_mesh(arrow_mesh);

	if (gd == Y_AXIS)
		set_rotation({0, 0, M_PI_2});
	if (gd == Z_AXIS)
		set_rotation({0, -M_PI_2, 0});

	Ref<CapsuleShape3D> shape = memnew(CapsuleShape3D);
	shape->set_radius(head_radius);
	shape->set_height(tip_offset);
	set_shape(shape, Transform3D(Basis(Vector3(0, 0, 1), M_PI_2), Vector3(tip_offset / 2, 0, 0)));
}

void Trans1DGizmo::interact(const Vector3& mouse_origin, const Vector3& mouse_normal, bool reset) {
	Vector3 global_direction = get_global_transform().basis.get_column(0).normalized();

	Vector3 n = mouse_normal.cross(mouse_normal.cross(global_direction));

	real_t skew_offset = (mouse_origin - get_global_transform().origin).dot(n) / global_direction.dot(n);

	if (reset) {
		pos_offset = skew_offset;
	} else {
		WRL::Change change;
		Vector3 pos_change = (skew_offset - pos_offset) * get_transform().basis.get_column(0).normalized();
		change.propertyChanges.insert({entry, position}, pos_change + wrl->get_entry_property(entry, position));
		wrl->submit_change(change);
	}
}

RotateGizmo::RotateGizmo(WRL::EntryID entry, String position, String p_rotation, GizmoDir gd)
	: Gizmo(entry, position, gd), rotation(p_rotation) {
	if (ring_mesh.is_null()) {
		const static Vector<Vector2> ring_profile = {
			{0, large_radius - small_radius},
			{-small_radius, large_radius},
			{0, large_radius + small_radius},
			{small_radius, large_radius},
			{0, large_radius - small_radius}};
		ring_mesh = build_rotation_solid(ring_profile, rot_segments);
		ring_shape = ring_mesh->create_trimesh_shape();
	}

	set_mesh(ring_mesh);
	set_shape(ring_shape);

	if (gd == Y_AXIS)
		set_rotation({0, 0, M_PI_2});
	if (gd == Z_AXIS)
		set_rotation({0, -M_PI_2, 0});
}

void RotateGizmo::interact(const Vector3& mouse_origin, const Vector3& mouse_normal, bool reset) {
	if (reset) {
		last_angle = plane_position(mouse_origin, mouse_normal).angle();
	} else {
		real_t current_angle = plane_position(mouse_origin, mouse_normal).angle();
		WRL::Change change;
		Quaternion rot_change(get_basis().get_column(0).normalized(), current_angle - last_angle);
		change.propertyChanges.insert(
			{entry, rotation}, rot_change * wrl->get_entry_property<Quaternion>(entry, rotation));
		last_angle = current_angle;
		wrl->submit_change(change);
	}
}