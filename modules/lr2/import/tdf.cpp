#include "tdf.h"

void TDF::load(const String& p_path) {
	path = p_path;
	String terr_data_path = path + "/TERRDATA.TDF";
	FileAccess* file = FileAccess::open(terr_data_path, FileAccess::READ);

	file->seek(0x10);
	height_scale = file->get_float();

	chunks.resize(num_chunks * num_chunks);
	for (int i = 0; i < chunks.size(); i++) {
		file->seek(0x3AE020 + i * 4);
		uint32_t surface_offset = file->get_32();

		Chunk chunk;

		file->seek(0x366020 + surface_offset + 3 * 4);
		chunk.pos_x = file->get_16();
		chunk.pos_y = file->get_16();

		file->seek(0x366020 + surface_offset + 0x90);
		uint32_t verticies_offset = file->get_32();

		file->seek(0x20 + verticies_offset);
		chunk.verticies.resize(vertex_chunk * vertex_chunk);
		for (int v = 0; v < chunk.verticies.size(); v++) {
			Chunk::Vertex vertex;

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

		chunks.set(i, chunk);
	}
}

TDFMesh::TDFMesh() {
	shader.instantiate();
	shader->set_code(R"(
		shader_type spatial;
		render_mode blend_mix, depth_draw_opaque, cull_back, diffuse_lambert, specular_disabled, vertex_lighting;
		varying vec4 mix;
		void vertex() {mix = CUSTOM0;}
		uniform sampler2D tex0 : hint_black_albedo;
		uniform sampler2D tex1 : hint_black_albedo;
		uniform sampler2D tex2 : hint_black_albedo;
		uniform sampler2D tex3 : hint_black_albedo;
		void fragment() {
			ALBEDO = (mat4(texture(tex0, UV),texture(tex1, UV),texture(tex2, UV),texture(tex3, UV)) * mix).rgb;
		}
	)");
}

struct TempSurface {
	Vector<Vector3> vertices;
	Vector<Vector3> normals;
	Vector<Vector2> uv;
	Vector<bool> cutout;
	Vector<uint8_t> mix;
};

void TDFMesh::rebuild_mesh() {
	clear_surfaces();
	if (tdf.is_null())
		return;

	Map<uint32_t, TempSurface> surfaces;

	for (int i = 0; i < tdf->chunks.size(); i++) {
		const TDF::Chunk& chunk = tdf->chunks[i];
		uint32_t mat_key = chunk.texture0 + (chunk.texture1 << 8) + (chunk.texture2 << 16) + (chunk.texture3 << 24);
		if (!surfaces.has(mat_key)) {
			surfaces.insert(mat_key, TempSurface());
		}
		TempSurface& temp_surface = surfaces.find(mat_key)->get();

		for (int sz = 0; sz < tdf->vertex_chunk; sz++) {
			for (int sx = 0; sx < tdf->vertex_chunk; sx++) {
				int index = sz * tdf->vertex_chunk + sx;
				const TDF::Chunk::Vertex& vertex = chunk.verticies[index];

				temp_surface.vertices.push_back(Vector3(
					chunk.pos_x + sx - tdf->chunk_width * tdf->num_chunks / 2, vertex.height * tdf->height_scale,
					chunk.pos_y + sz - tdf->chunk_width * tdf->num_chunks / 2));
				temp_surface.normals.push_back(Vector3(vertex.normal_x, vertex.normal_y, vertex.normal_z).normalized());
				temp_surface.uv.push_back(
					Vector2(
						static_cast<float>(chunk.pos_x + sx) / tdf->chunk_width,
						static_cast<float>(chunk.pos_y + sz) / tdf->chunk_width) *
					texture_scale);
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
		add_surface_from_arrays(
			Mesh::PRIMITIVE_TRIANGLES, array, Array(), Dictionary(),
			ARRAY_CUSTOM_RGBA8_UNORM << ARRAY_FORMAT_CUSTOM0_SHIFT);

		Ref<ShaderMaterial> mat = memnew(ShaderMaterial);
		mat->set_shader(shader);

		uint8_t texture0 = E.key;
		uint8_t texture1 = E.key >> 8;
		uint8_t texture2 = E.key >> 16;
		uint8_t texture3 = E.key >> 24;

		if (texture0 != 0xff) {
			mat->set_shader_param(
				"tex0", ResourceLoader::load(tdf->path + "/texture" + String::num_uint64(texture0 + 1) + ".tga"));
			if (texture1 != 0xff) {
				mat->set_shader_param(
					"tex1", ResourceLoader::load(tdf->path + "/texture" + String::num_uint64(texture1 + 1) + ".tga"));
				if (texture2 != 0xff) {
					mat->set_shader_param(
						"tex2",
						ResourceLoader::load(tdf->path + "/texture" + String::num_uint64(texture2 + 1) + ".tga"));
					if (texture3 != 0xff) {
						mat->set_shader_param(
							"tex3",
							ResourceLoader::load(tdf->path + "/texture" + String::num_uint64(texture3 + 1) + ".tga"));
					}
				}
			}
		}

		surface_set_material(get_surface_count() - 1, mat);
	}
}