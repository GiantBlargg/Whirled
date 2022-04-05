#pragma once

#include "../io/custom_file_dialog.hpp"
#include "../io/custom_fs.hpp"
#include "../wrl/wrl.hpp"
#include "scene_layout.h"
#include "viewer.h"

class Whirled : public Node {
	GDCLASS(Whirled, Node);

  private:
	const CustomFS custom_fs;
	String file_path = "";
	Ref<WRL> wrl;

	CustomFileDialog* file;

	Viewer* viewer;
	SceneLayout* scene;

	void _menu_new();
	void _menu_open();
	void _menu_save();
	void _menu_save_as();

	void _file_open(String path);
	void _file_save_as(String path);
	void _file_reset();

  public:
	Whirled(const CustomFS);
};