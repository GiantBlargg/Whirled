#include "register_types.h"

#include "image_loader_mip.h"

static ImageLoaderMIP *image_loader_mip = NULL;

void register_lr2_types() {

	image_loader_mip = memnew(ImageLoaderMIP);
	ImageLoader::add_image_format_loader(image_loader_mip);
}

void unregister_lr2_types() {

	memdelete(image_loader_mip);
}
