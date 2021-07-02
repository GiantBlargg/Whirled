#include "image_texture_loader.h"

#include "core/io/image_loader.h"
#include "scene/resources/texture.h"

RES ImageTextureLoader::load(
	const String& p_path, const String& p_original_path, Error* r_error, bool p_use_sub_threads, float* r_progress,
	CacheMode p_cache_mode) {

	Ref<Image> image;
	image.instantiate();

	if (!FileAccess::exists(p_path) && p_path.get_extension().to_lower() == "tga") {
		return ResourceLoader::load(p_path.get_basename() + ".mip", "Texture2D", p_cache_mode, r_error);
	}

	Error err = ImageLoader::load_image(p_path, image);

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

	Ref<ImageTexture> texture = memnew(ImageTexture);
	texture->create_from_image(image);

	if (r_error)
		*r_error = OK;

	return texture;
}

void ImageTextureLoader::get_recognized_extensions(List<String>* p_extensions) const {
	ImageLoader::get_recognized_extensions(p_extensions);
}

bool ImageTextureLoader::handles_type(const String& p_type) const {
	return ClassDB::is_parent_class("ImageTexture", p_type);
}

String ImageTextureLoader::get_resource_type(const String& p_path) const {
	String extension = p_path.get_extension().to_lower();
	List<String> extensions;
	ImageLoader::get_recognized_extensions(&extensions);

	for (auto e = extensions.front(); e; e = e->next()) {
		if (extension == e->get())
			return "ImageTexture";
	}
	return "";
}