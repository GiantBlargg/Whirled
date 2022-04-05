#include "ifl.hpp"

#include "core/io/file_access.h"
#include "modules/regex/regex.h"
#include "scene/resources/texture.h"

const float delay_time = 1.0f / 30.0f;

bool IFLAssetLoader::can_handle(const AssetKey& key, const CustomFS& fs) const {
	if (!ClassDB::is_parent_class("AnimatedTexture", key.type))
		return false;
	if (key.path.get_extension().to_lower() != "ifl")
		return false;
	return true;
}

AssetKey IFLAssetLoader::remap_key(const AssetKey& k, const CustomFS& fs) const { return {k.path, "AnimatedTexture"}; }

REF IFLAssetLoader::load(const AssetKey& k, const CustomFS& fs, AssetManager& assets, Error* r_error) const {
	auto dir_path = k.path.get_base_dir();

	String contents = fs.get_file_as_string(k.path);

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

		texture->set_frame_texture(i, assets.block_get<Texture2D>(tex_path));
		texture->set_frame_delay(i, delay_time * match->get_string(2).to_int());
	}

	return texture;
}
