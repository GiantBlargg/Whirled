#include "register_types.h"

#include "assets/tdf.hpp"
#include "core/config/engine.h"
#include "init.hpp"
#include "io/custom_file_dialog.hpp"
#include "io/image_loader_mip.h"

static ImageLoaderMIP* image_loader_mip = NULL;

void initialize_lr2_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	image_loader_mip = memnew(ImageLoaderMIP);
	ImageLoader::add_image_format_loader(image_loader_mip);

	ClassDB::register_class<Init>();
	ClassDB::register_class<CustomFileDialog>();
	ClassDB::register_class<TDF>();
}

extern Ref<Shader> tdf_shader;

void uninitialize_lr2_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	tdf_shader.unref();

	ImageLoader::remove_image_format_loader(image_loader_mip);
	memdelete(image_loader_mip);
}
