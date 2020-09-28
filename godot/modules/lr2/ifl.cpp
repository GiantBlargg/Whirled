#include "ifl.h"

#include "core/os/file_access.h"
#include "scene/resources/texture.h"
#include "modules/regex/regex.h"

const float delay_time = 1.0f / 30.0f;

RES IFLLoader::load(
	const String &p_path, const String &p_original_path,
	Error *r_error, bool p_use_sub_threads,
	float *r_progress, bool p_no_cache) {

	auto dir_path = p_path.get_base_dir();

	auto f = FileAccess::open(p_path, FileAccess::READ);
	auto contents = f->get_as_utf8_string();
	f->close();
	memdelete(f);

	Ref<AnimatedTexture> texture;
	texture.instance();
	texture->set_fps(0);

	Ref<RegEx> regex;
	regex.instance();
	regex->compile("(\\S+)\\s(\\d+)");
	auto matches = regex->search_all(contents);
	texture->set_frames(matches.size());
	for(int i = 0; i < matches.size(); i++) {
		Ref<RegExMatch> match = matches[i];

		String tex_path = dir_path + "/" + match->get_string(1);

		texture->set_frame_texture(i, ResourceLoader::load(tex_path));
		texture->set_frame_delay(i,delay_time * match->get_string(2).to_int());
	}

	return texture;
}

void IFLLoader::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("ifl");
}

bool IFLLoader::handles_type(const String &p_type) const {
	return p_type == "AnimatedTexture";
}

String IFLLoader::get_resource_type(const String &p_path) const {
	if (p_path.get_extension().to_lower() == "ifl")
		return "AnimatedTexture";
	return "";
}
