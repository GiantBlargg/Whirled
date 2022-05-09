#include "mesh_shape.hpp"

#include "scene/resources/mesh.h"

bool MeshShapeLoader::can_handle(const AssetKey& key, const CustomFS& fs) const {
	return ClassDB::is_parent_class("ConcavePolygonShape3D", key.type);
}
AssetKey MeshShapeLoader::remap_key(const AssetKey& key, const CustomFS&) const {
	return {key.path, "ConcavePolygonShape3D"};
}
REF MeshShapeLoader::load(const AssetKey& key, const CustomFS&, AssetManager& asset, Error*) const {
	auto mesh = asset.block_get<Mesh>(key.path);
	auto shape = mesh->create_trimesh_shape();
	return shape;
}