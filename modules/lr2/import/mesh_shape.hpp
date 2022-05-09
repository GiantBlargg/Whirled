#pragma once

#include "../io/asset_manager.hpp"

class MeshShapeLoader : public AssetLoader {
	GDCLASS(MeshShapeLoader, AssetLoader);

	bool can_handle(const AssetKey&, const CustomFS&) const override;
	AssetKey remap_key(const AssetKey&, const CustomFS&) const override;
	REF load(const AssetKey&, const CustomFS&, AssetManager&, Error*) const override;
};