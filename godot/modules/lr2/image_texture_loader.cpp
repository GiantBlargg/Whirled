#include "image_texture_loader.h"

#include "core/io/image_loader.h"

RES ImageTextureLoader::load(
	const String &p_path, const String &p_original_path,
	Error *r_error, bool p_use_sub_threads,
	float *r_progress, bool p_no_cache) {

	if (!FileAccess::exists(p_path)) {
		if (r_error) *r_error = ERR_FILE_NOT_FOUND;
		return NULL;
	}

	Ref<Image> image;
	image.instance();
	image->load(p_path);

	if (image->empty()) {
		if (r_error) *r_error = ERR_FILE_CANT_READ;
		return NULL;
	}

	image->flip_y();

	Ref<ImageTexture> texture = memnew(ImageTexture);
	texture->create_from_image(image);

	if (r_error)
		*r_error = OK;

	return texture;
}

void ImageTextureLoader::get_recognized_extensions(List<String> *p_extensions) const {
	ImageLoader::get_recognized_extensions(p_extensions);
}

bool ImageTextureLoader::handles_type(const String &p_type) const {
	return ClassDB::is_parent_class(p_type, "Texture2D");
}

String ImageTextureLoader::get_resource_type(const String &p_path) const {
	String extension = p_path.get_extension().to_lower();
	List<String> extensions;
	ImageLoader::get_recognized_extensions(&extensions);

	for(auto e = extensions.front(); e; e = e->next()){
		if(extension == e->get()) return "ImageTexture";
	}
	return "";
}