#pragma once

#include "lr2/io/custom_file_dialog.hpp"
#include "lr2/io/custom_fs.hpp"
#include "lr2/wrl/wrl.hpp"
#include "scene_layout.hpp"
#include "viewer/viewer.hpp"

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