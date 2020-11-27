#include "register_types.h"

#include "core/config/engine.h"
#include "lr2_dir.h"
#include "image_loader_mip.h"
#include "image_texture_loader.h"
#include "ifl.h"
#include "mdl2.h"

static LR2Dir* lr2_dir = NULL;
static ImageLoaderMIP* image_loader_mip = NULL;
static Ref<ImageTextureLoader> image_texture_loader;
static Ref<IFLLoader> ifl_loader;
static Ref<MDL2Loader> mdl2_loader;

void register_lr2_types() {
	ClassDB::register_class<LR2Dir>();
	lr2_dir = memnew(LR2Dir);
	lr2_dir->init();
	Engine::get_singleton()->add_singleton(Engine::Singleton("LR2Dir", LR2Dir::get_singleton()));

	image_loader_mip = memnew(ImageLoaderMIP);
	ImageLoader::add_image_format_loader(image_loader_mip);

	image_texture_loader.instance();
	ResourceLoader::add_resource_format_loader(image_texture_loader);

	ifl_loader.instance();
	ResourceLoader::add_resource_format_loader(ifl_loader);

	mdl2_loader.instance();
	ResourceLoader::add_resource_format_loader(mdl2_loader);
}

void unregister_lr2_types() {
	memdelete(lr2_dir);

	ImageLoader::remove_image_format_loader(image_loader_mip);
	memdelete(image_loader_mip);

	ResourceLoader::remove_resource_format_loader(image_texture_loader);
	image_texture_loader.unref();

	ResourceLoader::remove_resource_format_loader(ifl_loader);
	ifl_loader.unref();

	ResourceLoader::add_resource_format_loader(mdl2_loader);
	mdl2_loader.unref();
}
