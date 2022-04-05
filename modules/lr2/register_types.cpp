#include "register_types.h"

#include "core/config/engine.h"
#include "import/image_loader_mip.h"
#include "init.h"

static ImageLoaderMIP* image_loader_mip = NULL;

void register_lr2_types() {
	image_loader_mip = memnew(ImageLoaderMIP);
	ImageLoader::add_image_format_loader(image_loader_mip);

	ClassDB::register_class<Init>();
}

void unregister_lr2_types() {
	ImageLoader::remove_image_format_loader(image_loader_mip);
	memdelete(image_loader_mip);
}
