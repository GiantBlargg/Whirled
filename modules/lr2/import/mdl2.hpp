#pragma once

#include "../io/asset_manager.hpp"

class MDL2AssetLoader : public AssetLoader {
	GDCLASS(MDL2AssetLoader, AssetLoader);

	bool can_handle(const AssetKey&, const CustomFS&) const override;
	AssetKey remap_key(const AssetKey&, const CustomFS&) const override;
	REF load(const AssetKey&, const CustomFS&, AssetManager&, Error*) const override;
};
