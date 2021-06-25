#include "mdl2.h"

#include "core/io/file_access.h"
#include "scene/resources/mesh.h"

enum class MDL2Chunk : uint32_t {
	END = 0,
	MDL0 = 0x304c444d,
	MDL1 = 0x314c444d,
	MDL2 = 0x324c444d,
	GEO0 = 0x304f4547,
	GEO1 = 0x314f4547,
	P2G0 = 0x30473250,
	COLD = 0x444c4f43,
	SHA0 = 0x30414853,
};

#define FLAGS(T)                                                                                                       \
	inline T operator|(T lhs, T rhs) {                                                                                 \
		using U = std::underlying_type_t<T>;                                                                           \
		return static_cast<T>(static_cast<U>(lhs) | static_cast<U>(rhs));                                              \
	}                                                                                                                  \
	inline T operator&(T lhs, T rhs) {                                                                                 \
		using U = std::underlying_type_t<T>;                                                                           \
		return static_cast<T>(static_cast<U>(lhs) & static_cast<U>(rhs));                                              \
	}                                                                                                                  \
	inline bool has_flag(T flag, T test) { return (flag & test) == test; }

enum class VertexFlag : uint16_t { Vector = 1 << 0, Normal = 1 << 1, Colour = 1 << 2, UV = 1 << 3 };
FLAGS(VertexFlag)

struct Blend {
	uint32_t effect;
	uint16_t texture_id;
	uint8_t coordinate_index;
	uint8_t tiling;
};

struct MDL2Material {
	Color ambient;
	Color diffuse;
	Color specular;
	Color emissive;
	float shine;
	float alpha;
	uint32_t alpha_type;
	uint32_t bitfield;
	uint64_t anim_name;
};

inline Vector2 get_vector2(FileAccess* f) {
	float x = f->get_float();
	float y = f->get_float();
	return Vector2(x, y);
}
inline Vector3 get_vector3(FileAccess* f) {
	float x = f->get_float();
	float y = f->get_float();
	float z = f->get_float();
	return Vector3(x, y, z);
}
inline Color get_colour(FileAccess* f) {
	float r = f->get_float();
	float g = f->get_float();
	float b = f->get_float();
	float a = f->get_float();
	return Color(r, g, b, a);
}
inline String get_string(FileAccess* f, int length) {
	CharString cs;
	cs.resize(length + 1);
	f->get_buffer((uint8_t*)cs.ptr(), length);
	cs[length] = 0;

	String ret;
	ret.parse_utf8(cs.ptr());

	return ret;
}

void load_vertices(FileAccess* f, Array* group_arrays) {
	auto vertex_vector_offset = f->get_32();
	auto vertex_normal_offset = f->get_32();
	auto vertex_colour_offset = f->get_32();
	auto vertex_texcoord_offset = f->get_32();

	auto vertex_size = f->get_32();

	auto texcoord_count = f->get_32();

	VertexFlag flags = static_cast<VertexFlag>(f->get_16());
	auto vertices_count = f->get_16();

	auto start_position = f->get_position() + 12;

	Vector<Vector3> vectors;
	vectors.resize(vertices_count);
	Vector<Vector3> normals;
	normals.resize(vertices_count);
	Vector<Color> colours;
	colours.resize(vertices_count);
	Vector<Vector2> uv;
	uv.resize(vertices_count);
	Vector<Vector2> uv2;
	uv2.resize(vertices_count);

	for (int vertex = 0; vertex < vertices_count; vertex++) {
		if (has_flag(flags, VertexFlag::Vector)) {
			f->seek(start_position + vertex * vertex_size + vertex_vector_offset);
			vectors.set(vertex, get_vector3(f));
		}
		if (has_flag(flags, VertexFlag::Normal)) {
			f->seek(start_position + vertex * vertex_size + vertex_normal_offset);
			normals.set(vertex, get_vector3(f));
		}
		if (has_flag(flags, VertexFlag::Colour)) {
			f->seek(start_position + vertex * vertex_size + vertex_colour_offset);
			colours.set(vertex, get_colour(f));
		}
		if (has_flag(flags, VertexFlag::UV)) {
			f->seek(start_position + vertex * vertex_size + vertex_texcoord_offset);
			uv.set(vertex, get_vector2(f));
			if (texcoord_count > 1) {
				uv2.set(vertex, get_vector2(f));
				// Godot only supports 2 sets of uv
				// Shouldn't matter as LR2 probably doesn't either
			}
		}
	}

	if (has_flag(flags, VertexFlag::Vector)) {
		group_arrays->set(Mesh::ArrayType::ARRAY_VERTEX, vectors);
	}
	if (has_flag(flags, VertexFlag::Normal)) {
		group_arrays->set(Mesh::ArrayType::ARRAY_NORMAL, normals);
	}
	if (has_flag(flags, VertexFlag::Colour)) {
		group_arrays->set(Mesh::ArrayType::ARRAY_COLOR, colours);
	}
	if (has_flag(flags, VertexFlag::UV)) {
		group_arrays->set(Mesh::ArrayType::ARRAY_TEX_UV, uv);
		if (texcoord_count > 1) {
			group_arrays->set(Mesh::ArrayType::ARRAY_TEX_UV2, uv2);
		}
	}

	f->seek(start_position + vertices_count * vertex_size);
}

RES MDL2Loader::load(
	const String& p_path, const String& p_original_path, Error* r_error, bool p_use_sub_threads, float* r_progress,
	CacheMode p_cache_mode) {

	FileAccess* f = FileAccess::open(p_path, FileAccess::READ);

	Ref<ArrayMesh> mesh;
	mesh.instantiate();

	Vector<String> textures;
	Vector<MDL2Material> material_props;

	while (true) {
		MDL2Chunk type = static_cast<MDL2Chunk>(f->get_32());
		uint32_t chunk_size = f->get_32();

		uint32_t next_chunk = f->get_position() + chunk_size;

		switch (type) {
		default:
			print_error("Unknown chunk");
			break;

		case MDL2Chunk::MDL1:
		case MDL2Chunk::MDL2: {
			f->seek(f->get_position() + 12 + 8);
			uint32_t has_bounding_box = f->get_32();
			if (has_bounding_box)
				f->seek(f->get_position() + 12 + 12 + 12 + 4);
			f->seek(f->get_position() + 16 + 48);

			textures.resize(f->get_32());

			auto texture_start = f->get_position();

			for (int i = 0; i < textures.size(); i++) {
				f->seek(texture_start + i * (256 + 8));
				auto path = get_string(f, 256);
				if (p_use_sub_threads)
					ResourceLoader::load_threaded_request("lr2://" + path);
				textures.set(i, path);
			}

			f->seek(texture_start + textures.size() * (256 + 8));

			material_props.resize(f->get_32());
			for (int i = 0; i < material_props.size(); i++) {
				MDL2Material m;
				if (type == MDL2Chunk::MDL2) {
					m.ambient = get_colour(f);
					m.diffuse = get_colour(f);
					m.specular = get_colour(f);
					m.emissive = get_colour(f);
					m.shine = f->get_float();
					m.alpha = f->get_float();
					m.alpha_type = f->get_32();
					m.bitfield = f->get_32();
					m.anim_name = f->get_64();
				} else {
					m.alpha_type = f->get_32();
					auto u1 = f->get_float();
					auto u2 = f->get_float();
					auto u3 = f->get_float();
					auto u4 = f->get_float();
					auto u5 = f->get_float();
					auto u6 = f->get_float();
				}
				material_props.set(i, m);
			}
		} break;
		case MDL2Chunk::GEO1: {
			auto detail_level_count = f->get_32();

			uint32_t detail_level_type = f->get_32();
			f->get_float();
			auto render_group_count = f->get_32();
			f->get_64();

			for (int render_group = 0; render_group < render_group_count; render_group++) {
				f->seek(f->get_position() + 4);
				auto material_id = f->get_16();
				f->seek(f->get_position() + 2 + 12 + 8);

				Vector<Blend> blends;
				blends.resize(4);
				for (int i = 0; i < blends.size(); i++) {
					Blend b;
					b.effect = f->get_32();
					b.texture_id = f->get_16();
					b.coordinate_index = f->get_8();
					b.tiling = f->get_8();
					blends.set(i, b);
				}

				Array group_arrays;
				group_arrays.resize(Mesh::ArrayType::ARRAY_MAX);

				load_vertices(f, &group_arrays);

				f->get_32();
				auto fill_type = f->get_32();

				Vector<int> indicies;
				indicies.resize(f->get_32());
				for (int index = 0; index < indicies.size(); index++) {
					indicies.set(index, f->get_16());
				}

				group_arrays.set(Mesh::ArrayType::ARRAY_INDEX, indicies);

				if (fill_type == 0)
					mesh->add_surface_from_arrays(Mesh::PrimitiveType::PRIMITIVE_TRIANGLES, group_arrays);
				else
					mesh->add_surface_from_arrays(Mesh::PrimitiveType::PRIMITIVE_TRIANGLE_STRIP, group_arrays);

				auto mat_prop = material_props.get(material_id);

				Ref<StandardMaterial3D> mat;
				mat.instantiate();

				mat->set_cull_mode(BaseMaterial3D::CullMode::CULL_FRONT);
				mat->set_diffuse_mode(BaseMaterial3D::DiffuseMode::DIFFUSE_LAMBERT);
				mat->set_specular_mode(BaseMaterial3D::SpecularMode::SPECULAR_DISABLED);

				float alpha = 1 - mat_prop.alpha;

				switch (mat_prop.alpha_type) {
				case 0:
					mat->set_transparency(BaseMaterial3D::Transparency::TRANSPARENCY_ALPHA_SCISSOR);
					mat->set_depth_draw_mode(BaseMaterial3D::DepthDrawMode::DEPTH_DRAW_ALWAYS);
					mat->set_alpha_scissor_threshold(0.5f);
					break;
				case 1:
					mat->set_blend_mode(BaseMaterial3D::BlendMode::BLEND_MODE_MIX);
					mat->set_transparency(BaseMaterial3D::Transparency::TRANSPARENCY_ALPHA);
					break;
				case 4:
					mat->set_blend_mode(BaseMaterial3D::BlendMode::BLEND_MODE_ADD);
					mat->set_cull_mode(BaseMaterial3D::CullMode::CULL_DISABLED);
					mat->set_shading_mode(BaseMaterial3D::ShadingMode::SHADING_MODE_UNSHADED);
					break;

				default:
					print_error("Unkown Alpha Type " + itos(mat_prop.alpha_type) + " in " + p_path);
					break;
				}

				mat->set_albedo(Color(1, 1, 1, alpha));

				mat->set_texture(
					BaseMaterial3D::TextureParam::TEXTURE_ALBEDO,
					ResourceLoader::load_threaded_get("lr2://" + textures.get(blends.get(0).texture_id)));

				mesh->surface_set_material(render_group, mat);
			}

		} break;
		case MDL2Chunk::END: {
			f->close();
			memdelete(f);
			return mesh;
		} break;
		case MDL2Chunk::MDL0:
		case MDL2Chunk::GEO0:
		case MDL2Chunk::P2G0:
		case MDL2Chunk::COLD:
		case MDL2Chunk::SHA0:
			break;
		}
		f->seek(next_chunk);
	}
}

void MDL2Loader::get_recognized_extensions(List<String>* p_extensions) const { p_extensions->push_back("md2"); }

bool MDL2Loader::handles_type(const String& p_type) const { return p_type == "Mesh"; }

String MDL2Loader::get_resource_type(const String& p_path) const {
	if (p_path.get_extension().to_lower() == "md2")
		return "Mesh";
	return "";
}