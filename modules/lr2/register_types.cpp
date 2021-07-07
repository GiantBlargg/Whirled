#include "register_types.h"

#include "core/config/engine.h"
#include "import/ifl.h"
#include "import/image_loader_mip.h"
#include "import/image_texture_loader.h"
#include "import/mdl2.h"
#include "init.h"

static ImageLoaderMIP* image_loader_mip = NULL;
static Ref<ImageTextureLoader> image_texture_loader;
static Ref<IFLLoader> ifl_loader;
static Ref<MDL2Loader> mdl2_loader;

void register_lr2_types() {

	image_loader_mip = memnew(ImageLoaderMIP);
	ImageLoader::add_image_format_loader(image_loader_mip);

	image_texture_loader.instantiate();
	ResourceLoader::add_resource_format_loader(image_texture_loader);

	ifl_loader.instantiate();
	ResourceLoader::add_resource_format_loader(ifl_loader);

	mdl2_loader.instantiate();
	ResourceLoader::add_resource_format_loader(mdl2_loader);

	ClassDB::register_class<Init>();
}

void unregister_lr2_types() {
	ImageLoader::remove_image_format_loader(image_loader_mip);
	memdelete(image_loader_mip);

	ResourceLoader::remove_resource_format_loader(image_texture_loader);
	image_texture_loader.unref();

	ResourceLoader::remove_resource_format_loader(ifl_loader);
	ifl_loader.unref();

	ResourceLoader::remove_resource_format_loader(mdl2_loader);
	mdl2_loader.unref();
}
