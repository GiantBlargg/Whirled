#pragma once

#include "scene/main/node.h"

class Init : public Node {
	GDCLASS(Init, Node);

  protected:
	void _notification(int p_notification);

  public:
	Init();
};