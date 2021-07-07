#pragma once

#include "scene/gui/file_dialog.h"
#include "scene_layout.h"
#include "viewer.h"
#include "wrl.h"

class Whirled : public Node {
	GDCLASS(Whirled, Node);

  private:
	String file_path;
	Ref<WRL> wrl;

	FileDialog* file;

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
	Whirled();
};