#include "register_types.h"

#include "core/config/engine.h"
#include "import/image_loader_mip.h"
#include "import/tdf.h"
#include "init.h"
#include "io/custom_file_dialog.hpp"

static ImageLoaderMIP* image_loader_mip = NULL;

void register_lr2_types() {
	image_loader_mip = memnew(ImageLoaderMIP);
	ImageLoader::add_image_format_loader(image_loader_mip);

	ClassDB::register_class<Init>();
	ClassDB::register_class<CustomFileDialog>();
	ClassDB::register_class<TDF>();
}

extern Ref<Shader> tdf_shader;

void unregister_lr2_types() {
	tdf_shader.unref();

	ImageLoader::remove_image_format_loader(image_loader_mip);
	memdelete(image_loader_mip);
}
