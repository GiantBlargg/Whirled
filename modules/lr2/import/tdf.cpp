#include "tdf.h"

bool TDFLoader::can_handle(const AssetKey& key, const CustomFS& fs) const {
	if (!ClassDB::is_parent_class("TDF", key.type))
		return false;
	if (key.path.get_extension().to_lower() != "")
		return false;
	return true;
}

AssetKey TDFLoader::remap_key(const AssetKey& k, const CustomFS& fs) const { return {k.path, "TDF"}; }

Ref<RefCounted> TDFLoader::load(const AssetKey& k, const CustomFS& fs, AssetManager& assets, Error* r_error) const {
	Ref<TDF> tdf;
	tdf.instantiate();

	tdf->path = k.path;
	String terr_data_path = k.path + "/TERRDATA.TDF";
	Ref<FileAccess> file = fs.FileAccess_open(terr_data_path, FileAccess::READ);

	file->seek(0x10);
	tdf->height_scale = file->get_float();

	tdf->chunks.resize(tdf->num_chunks * tdf->num_chunks);
	for (int i = 0; i < tdf->chunks.size(); i++) {
		file->seek(0x3AE020 + i * 4);
		uint32_t surface_offset = file->get_32();

		TDF::Chunk chunk;

		file->seek(0x366020 + surface_offset + 3 * 4);
		chunk.pos_x = file->get_16();
		chunk.pos_y = file->get_16();

		file->seek(0x366020 + surface_offset + 0x90);
		uint32_t verticies_offset = file->get_32();

		file->seek(0x20 + verticies_offset);
		chunk.verticies.resize(tdf->vertex_chunk * tdf->vertex_chunk);
		for (int v = 0; v < chunk.verticies.size(); v++) {
			TDF::Chunk::Vertex vertex;

			vertex.height = file->get_16();
			vertex.normal_x = file->get_8();
			vertex.normal_y = file->get_8();
			vertex.normal_z = file->get_8();
			vertex.flags = file->get_8();
			vertex.mix_ratios = file->get_16();

			chunk.verticies.set(v, vertex);
		}

		file->seek(0x366020 + surface_offset + 0x118);
		chunk.texture0 = file->get_8();
		chunk.texture1 = file->get_8();
		chunk.texture2 = file->get_8();
		chunk.texture3 = file->get_8();

		tdf->chunks.set(i, chunk);
	}

	return tdf;
}
bool TDFMeshLoader::can_handle(const AssetKey& k, const CustomFS&) const {
	if (!ClassDB::is_parent_class("ArrayMesh", k.type))
		return false;
	if (k.path.get_extension().to_lower() != "")
		return false;
	return true;
}

AssetKey TDFMeshLoader::remap_key(const AssetKey& k, const CustomFS&) const { return {k.path, "ArrayMesh"}; }

Ref<Shader> tdf_shader;

struct TempSurface {
	Vector<Vector3> vertices;
	Vector<Vector3> normals;
	Vector<Vector2> uv;
	Vector<bool> cutout;
	Vector<uint8_t> mix;
};

Ref<RefCounted> TDFMeshLoader::load(const AssetKey& k, const CustomFS&, AssetManager& assets, Error*) const {
	if (tdf_shader.is_null()) {
		tdf_shader.instantiate();
		tdf_shader->set_code(R"(
		shader_type spatial;
		render_mode blend_mix, depth_draw_opaque, cull_back, diffuse_lambert, specular_disabled, vertex_lighting;
		varying vec4 mix;
		void vertex() {mix = CUSTOM0;}
		uniform sampler2D tex0 : hint_black_albedo;
		uniform sampler2D tex1 : hint_black_albedo;
		uniform sampler2D tex2 : hint_black_albedo;
		uniform sampler2D tex3 : hint_black_albedo;
		instance uniform vec2 texture_scale;
		void fragment() {
			vec2 scaled_uv = UV * texture_scale;
			ALBEDO = (mat4(
				texture(tex0, scaled_uv),
				texture(tex1, scaled_uv),
				texture(tex2, scaled_uv),
				texture(tex3, scaled_uv)
			) * mix).rgb;
		}
	)");
	}

	Ref<ArrayMesh> mesh;
	mesh.instantiate();

	Ref<TDF> tdf = assets.block_get<TDF>(k.path);

	HashMap<uint32_t, TempSurface> surfaces;

	for (int i = 0; i < tdf->chunks.size(); i++) {
		const TDF::Chunk& chunk = tdf->chunks[i];
		uint32_t mat_key = chunk.texture0 + (chunk.texture1 << 8) + (chunk.texture2 << 16) + (chunk.texture3 << 24);
		if (!surfaces.has(mat_key)) {
			surfaces.insert(mat_key, TempSurface());
		}
		TempSurface& temp_surface = surfaces.find(mat_key)->value;

		for (int sz = 0; sz < tdf->vertex_chunk; sz++) {
			for (int sx = 0; sx < tdf->vertex_chunk; sx++) {
				int index = sz * tdf->vertex_chunk + sx;
				const TDF::Chunk::Vertex& vertex = chunk.verticies[index];

				temp_surface.vertices.push_back(Vector3(
					chunk.pos_x + sx - tdf->chunk_width * tdf->num_chunks / 2, vertex.height * tdf->height_scale,
					chunk.pos_y + sz - tdf->chunk_width * tdf->num_chunks / 2));
				temp_surface.normals.push_back(Vector3(vertex.normal_x, vertex.normal_y, vertex.normal_z).normalized());
				temp_surface.uv.push_back(Vector2(
					static_cast<float>(chunk.pos_x + sx) / tdf->chunk_width,
					static_cast<float>(chunk.pos_y + sz) / tdf->chunk_width));
				temp_surface.cutout.push_back((vertex.flags & 0b10000000) == 0b10000000);
				temp_surface.mix.push_back(((vertex.mix_ratios >> 0x0) & 0xf) * 0x11);
				temp_surface.mix.push_back(((vertex.mix_ratios >> 0x4) & 0xf) * 0x11);
				temp_surface.mix.push_back(((vertex.mix_ratios >> 0x8) & 0xf) * 0x11);
				temp_surface.mix.push_back(((vertex.mix_ratios >> 0xc) & 0xf) * 0x11);
			}
		}
	}

	for (auto& E : surfaces) {

		Vector<int> indices;
		for (int c = 0; c < E.value.vertices.size() / tdf->vertex_chunk / tdf->vertex_chunk; c++) {
			for (int z = 0; z < tdf->chunk_width; z++) {
				for (int x = 0; x < tdf->chunk_width; x++) {
					int base_vertex = (c * tdf->vertex_chunk * tdf->vertex_chunk) + (z * tdf->vertex_chunk + x);

					if (!E.value.cutout[base_vertex + 1 + tdf->vertex_chunk]) {
						if (z % 2 == 0) {
							indices.push_back(base_vertex);
							indices.push_back(base_vertex + 1);
							indices.push_back(base_vertex + tdf->vertex_chunk);
							indices.push_back(base_vertex + tdf->vertex_chunk);
							indices.push_back(base_vertex + 1);
							indices.push_back(base_vertex + 1 + tdf->vertex_chunk);
						} else {
							indices.push_back(base_vertex);
							indices.push_back(base_vertex + 1);
							indices.push_back(base_vertex + 1 + tdf->vertex_chunk);
							indices.push_back(base_vertex);
							indices.push_back(base_vertex + 1 + tdf->vertex_chunk);
							indices.push_back(base_vertex + tdf->vertex_chunk);
						}
					}
				}
			}
		}

		Array array;
		array.resize(ArrayMesh::ARRAY_MAX);
		array.set(ArrayMesh::ARRAY_VERTEX, E.value.vertices);
		array.set(ArrayMesh::ARRAY_NORMAL, E.value.normals);
		array.set(ArrayMesh::ARRAY_TEX_UV, E.value.uv);
		array.set(ArrayMesh::ARRAY_CUSTOM0, E.value.mix);
		array.set(ArrayMesh::ARRAY_INDEX, indices);
		mesh->add_surface_from_arrays(
			Mesh::PRIMITIVE_TRIANGLES, array, Array(), Dictionary(),
			Mesh::ARRAY_CUSTOM_RGBA8_UNORM << Mesh::ARRAY_FORMAT_CUSTOM0_SHIFT);

		Ref<ShaderMaterial> mat = memnew(ShaderMaterial);
		mat->set_shader(tdf_shader);

		uint8_t texture0 = E.key;
		uint8_t texture1 = E.key >> 8;
		uint8_t texture2 = E.key >> 16;
		uint8_t texture3 = E.key >> 24;

		if (texture0 != 0xff) {
			mat->set_shader_param(
				"tex0",
				assets.block_get<Texture2D>(tdf->path + "/texture" + String::num_uint64(texture0 + 1) + ".tga"));
			if (texture1 != 0xff) {
				mat->set_shader_param(
					"tex1",
					assets.block_get<Texture2D>(tdf->path + "/texture" + String::num_uint64(texture1 + 1) + ".tga"));
				if (texture2 != 0xff) {
					mat->set_shader_param(
						"tex2",
						assets.block_get<Texture2D>(
							tdf->path + "/texture" + String::num_uint64(texture2 + 1) + ".tga"));
					if (texture3 != 0xff) {
						mat->set_shader_param(
							"tex3",
							assets.block_get<Texture2D>(
								tdf->path + "/texture" + String::num_uint64(texture3 + 1) + ".tga"));
					}
				}
			}
		}

		mesh->surface_set_material(mesh->get_surface_count() - 1, mat);
	}
	return mesh;
}
