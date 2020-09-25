#include "register_types.h"

#include "image_loader_mip.h"
#include "image_texture_loader.h"
#include "lr2_dir.h"

static ImageLoaderMIP* image_loader_mip = NULL;
static Ref<ImageTextureLoader> image_texture_loader;
static LR2Dir* lr2_dir = NULL;

void register_lr2_types() {
	image_loader_mip = memnew(ImageLoaderMIP);
	ImageLoader::add_image_format_loader(image_loader_mip);

	image_texture_loader.instance();
	ResourceLoader::add_resource_format_loader(image_texture_loader);

	ClassDB::register_class<LR2Dir>();
	lr2_dir = memnew(LR2Dir);
	lr2_dir->init();
	Engine::get_singleton()->add_singleton(Engine::Singleton("LR2Dir", LR2Dir::get_singleton()));
}

void unregister_lr2_types() {
	ImageLoader::remove_image_format_loader(image_loader_mip);
	memdelete(image_loader_mip);

	ResourceLoader::remove_resource_format_loader(image_texture_loader);
	image_texture_loader.unref();

	memdelete(lr2_dir);
}
