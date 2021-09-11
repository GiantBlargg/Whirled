#include "ifl.h"

#include "core/io/file_access.h"
#include "modules/regex/regex.h"
#include "scene/resources/texture.h"

const float delay_time = 1.0f / 30.0f;

RES IFLLoader::load(
	const String& p_path, const String& p_original_path, Error* r_error, bool p_use_sub_threads, float* r_progress,
	CacheMode p_cache_mode) {

	auto dir_path = p_path.get_base_dir();

	String contents = FileAccess::get_file_as_string(p_path);

	Ref<RegEx> regex;
	regex.instantiate();
	regex->compile("(\\S+)\\s(\\d+)");
	auto matches = regex->search_all(contents);

	Ref<AnimatedTexture> texture;
	texture.instantiate();
	texture->set_fps(0);

	texture->set_frames(matches.size());
	for (int i = 0; i < matches.size(); i++) {
		Ref<RegExMatch> match = matches[i];

		String tex_path = dir_path + "/" + match->get_string(1);

		texture->set_frame_texture(i, ResourceLoader::load(tex_path, "Texture2D", p_cache_mode, r_error));
		texture->set_frame_delay(i, delay_time * match->get_string(2).to_int());
	}

	return texture;
}

void IFLLoader::get_recognized_extensions(List<String>* p_extensions) const { p_extensions->push_back("ifl"); }

bool IFLLoader::handles_type(const String& p_type) const { return ClassDB::is_parent_class("AnimatedTexture", p_type); }

String IFLLoader::get_resource_type(const String& p_path) const {
	if (p_path.get_extension().to_lower() == "ifl")
		return "AnimatedTexture";
	return "";
}
