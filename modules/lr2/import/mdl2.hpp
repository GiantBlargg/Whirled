#pragma once

#include "../io/asset_manager.hpp"

class MDL2AssetLoader : public AssetLoader {
	GDCLASS(MDL2AssetLoader, AssetLoader);

	bool can_handle(const AssetKey& key, const CustomFS& fs) const override;
	AssetKey remap_key(const AssetKey& k, const CustomFS& fs) const override;
	REF load(const AssetKey& k, const CustomFS&, AssetManager& assets, Error* r_error) const override;
};
