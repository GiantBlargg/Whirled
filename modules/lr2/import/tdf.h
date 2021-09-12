#pragma once

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

	void load(const String& p_path);
};

class TDFMesh : public ArrayMesh {
	GDCLASS(TDFMesh, ArrayMesh);

  private:
	Ref<Shader> shader;

	Ref<TDF> tdf;
	Vector2 texture_scale;

	mutable RID mesh;

	void rebuild_mesh();

  public:
	TDFMesh();

	const Vector2& get_texture_scale() { return texture_scale; }
	void set_texture_scale(const Vector2& p_texture_scale) {
		if (texture_scale == p_texture_scale)
			return;
		texture_scale = p_texture_scale;
		rebuild_mesh();
	}

	const Ref<TDF>& get_tdf() { return tdf; }
	void set_tdf(const Ref<TDF>& p_tdf) {
		if (tdf == p_tdf)
			return;
		tdf = p_tdf;
		rebuild_mesh();
	}
};