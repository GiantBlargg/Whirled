#include "image_asset_loader.hpp"

#include "core/io/image_loader.h"
#include "scene/resources/texture.h"

bool ImageAssetLoader::can_handle(const AssetKey& key, const CustomFS& fs) const {
	if (!ClassDB::is_parent_class("Image", key.type))
		return false;

	String extension = key.path.get_extension().to_lower();
	List<String> extensions;
	ImageLoader::get_recognized_extensions(&extensions);
	bool handles_file_type = false;
	for (auto e = extensions.front(); e; e = e->next()) {
		if (extension == e->get()) {
			handles_file_type = true;
			break;
		}
	}
	return handles_file_type;
}
AssetKey ImageAssetLoader::remap_key(const AssetKey& k, const CustomFS& fs) const {
	String path = k.path;
	if (!fs.file_exists(path) && path.get_extension().to_lower() == "tga") {
		String try_path = path.get_basename() + ".mip";
		if (fs.file_exists(try_path)) {
			path = fs.canon_path(try_path);
		}
	}
	return {path, "Image"};
}
Ref<RefCounted> ImageAssetLoader::load(const AssetKey& k, const CustomFS& fs, AssetManager&, Error* r_error) const {
	Ref<Image> image;
	image.instantiate();
	Error err;
	{
		Ref<FileAccess> f = fs.FileAccess_open(k.path, FileAccess::READ, &err);
		if (err) {
			if (r_error)
				*r_error = err;
			return NULL;
		}

		err = ImageLoader::load_image(k.path, image, f);
	}

	if (err) {
		if (r_error)
			*r_error = err;
		return NULL;
	}

	if (image->is_empty()) {
		if (r_error)
			*r_error = ERR_FILE_CANT_READ;
		return NULL;
	}

	image->flip_y();

	if (r_error)
		*r_error = OK;

	return image;
}

bool ImageTextureLoader::can_handle(const AssetKey& key, const CustomFS& fs) const {
	if (!ClassDB::is_parent_class("ImageTexture", key.type))
		return false;

	String extension = key.path.get_extension().to_lower();
	List<String> extensions;
	ImageLoader::get_recognized_extensions(&extensions);
	bool handles_file_type = false;
	for (auto e = extensions.front(); e; e = e->next()) {
		if (extension == e->get()) {
			handles_file_type = true;
			break;
		}
	}
	return handles_file_type;
}
AssetKey ImageTextureLoader::remap_key(const AssetKey& k, const CustomFS& fs) const { return {k.path, "ImageTexture"}; }
Ref<RefCounted>
ImageTextureLoader::load(const AssetKey& k, const CustomFS&, AssetManager& assets, Error* r_error) const {
	Ref<ImageTexture> texture = memnew(ImageTexture);
	Ref<Image> image = assets.block_get<Image>(k.path);
	texture->create_from_image(image);

	if (r_error)
		*r_error = OK;

	return texture;
}