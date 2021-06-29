#pragma once

#include "scene/gui/file_dialog.h"
#include "scene/main/node.h"
#include "wrl.h"

class Whirled : public Node {
	GDCLASS(Whirled, Node);

  private:
	String file_path;
	Ref<WRL> wrl;

	FileDialog* file;

	void _menu_new();
	void _menu_open();
	void _menu_save();
	void _menu_save_as();

	void _file_open(String path);
	void _file_save_as(String path);
	void _file_close();

	void _update();

  public:
	Whirled();
};