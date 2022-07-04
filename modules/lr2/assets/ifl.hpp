#pragma once

#include "asset_manager.hpp"

class IFLLoader : public AssetLoader {
	GDCLASS(IFLLoader, AssetLoader);

	bool can_handle(const AssetKey&, const CustomFS&) const override;
	AssetKey remap_key(const AssetKey&, const CustomFS&) const override;
	Ref<RefCounted> load(const AssetKey&, const CustomFS&, AssetManager&, Error*) const override;
};
