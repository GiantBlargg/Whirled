#pragma once

#include "../io/asset_manager.hpp"
#include "../io/custom_fs.hpp"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/resources/mesh.h"

class TDF : public RefCounted {
	GDCLASS(TDF, RefCounted);

  public:
	String path;

	float height_scale;

	const int chunk_width = 16;
	const int vertex_chunk = chunk_width + 1;
	const int num_chunks = 32;

	struct Chunk {
		uint16_t pos_x;
		uint16_t pos_y;

		struct Vertex {
			uint16_t height;
			int8_t normal_x;
			int8_t normal_y;
			int8_t normal_z;
			uint8_t flags;
			uint16_t mix_ratios;
		};
		Vector<Vertex> verticies;

		uint8_t texture0;
		uint8_t texture1;
		uint8_t texture2;
		uint8_t texture3;
	};

	Vector<Chunk> chunks;
};

class TDFLoader : public AssetLoader {
	GDCLASS(TDFLoader, AssetLoader);

	bool can_handle(const AssetKey&, const CustomFS&) const override;
	AssetKey remap_key(const AssetKey&, const CustomFS&) const override;
	REF load(const AssetKey&, const CustomFS&, AssetManager&, Error*) const override;
};

class TDFMeshLoader : public AssetLoader {
	GDCLASS(TDFMeshLoader, AssetLoader);

	bool can_handle(const AssetKey&, const CustomFS&) const override;
	AssetKey remap_key(const AssetKey&, const CustomFS&) const override;
	REF load(const AssetKey&, const CustomFS&, AssetManager&, Error*) const override;
};
