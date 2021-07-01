#pragma once

#include "scene/main/node.h"

class Init : public Node {
	GDCLASS(Init, Node);

  private:
	DisplayServer::WindowID window;

  protected:
	void _notification(int p_notification);

  public:
	Init();
};