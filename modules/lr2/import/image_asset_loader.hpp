#pragma once

#include "../io/asset_manager.hpp"

class ImageAssetLoader : public AssetLoader {
	GDCLASS(ImageAssetLoader, AssetLoader);
	bool can_handle(const AssetKey&, const CustomFS&) const override;
	AssetKey remap_key(const AssetKey&, const CustomFS&) const override;
	Ref<RefCounted> load(const AssetKey&, const CustomFS&, AssetManager&, Error*) const override;
};

class ImageTextureLoader : public AssetLoader {
	GDCLASS(ImageTextureLoader, AssetLoader);
	bool can_handle(const AssetKey&, const CustomFS&) const override;
	AssetKey remap_key(const AssetKey&, const CustomFS& fs) const override;
	Ref<RefCounted> load(const AssetKey&, const CustomFS&, AssetManager&, Error*) const override;
};
