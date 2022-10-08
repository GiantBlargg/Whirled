#pragma once

enum Layer {
	LayerProps = 1 << 0,
	LayerTerrain = 1 << 1,
	LayerSkyBox = 1 << 2,
	LayerSelect = 1 << 3,
	LayerSkySelect = 1 << 4,
	LayerGizmo = 1 << 5
};