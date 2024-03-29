#include "ifl.hpp"

#include "core/io/file_access.h"
#include "modules/regex/regex.h"
#include "scene/resources/texture.h"

bool IFLLoader::can_handle(const AssetKey& key, const CustomFS& fs) const {
	if (!ClassDB::is_parent_class("AnimatedTexture", key.type))
		return false;
	if (key.path.get_extension().to_lower() != "ifl")
		return false;
	return true;
}

AssetKey IFLLoader::remap_key(const AssetKey& k, const CustomFS& fs) const { return {k.path, "AnimatedTexture"}; }

Ref<RefCounted> IFLLoader::load(const AssetKey& k, const CustomFS& fs, AssetManager& assets, Error* r_error) const {
	auto dir_path = k.path.get_base_dir();

	String contents = fs.get_file_as_string(k.path);

	Ref<RegEx> regex;
	regex.instantiate();
	regex->compile("(\\S+)\\s(\\d+)");
	auto matches = regex->search_all(contents);

	Ref<AnimatedTexture> texture;
	texture.instantiate();
	texture->set_speed_scale(30);

	texture->set_frames(matches.size());
	for (int i = 0; i < matches.size(); i++) {
		Ref<RegExMatch> match = matches[i];

		String tex_path = dir_path + "/" + match->get_string(1);

		texture->set_frame_texture(i, assets.block_get<Texture2D>(tex_path));
		texture->set_frame_duration(i, match->get_string(2).to_int());
	}

	return texture;
}
