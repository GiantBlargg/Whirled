#include "register_types.h"

#include "core/config/engine.h"
#include "import/image_loader_mip.h"
#include "import/tdf.h"
#include "init.h"
#include "io/custom_file_dialog.hpp"

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
