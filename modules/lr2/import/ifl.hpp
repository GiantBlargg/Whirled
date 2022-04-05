#pragma once

#include "../io/asset_manager.hpp"

class IFLAssetLoader : public AssetLoader {
	GDCLASS(IFLAssetLoader, AssetLoader);

	bool can_handle(const AssetKey& key, const CustomFS& fs) const override;
	AssetKey remap_key(const AssetKey& k, const CustomFS& fs) const override;
	REF load(const AssetKey& k, const CustomFS&, AssetManager& assets, Error* r_error) const override;
};
