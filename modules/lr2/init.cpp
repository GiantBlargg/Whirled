#include "init.hpp"

#include "core/string/translation.h"
#include "servers/navigation_server_3d.h"
#include "servers/physics_server_2d.h"

#include "editor/whirled.hpp"

void Init::_notification(int p_notification) {
	if (p_notification == NOTIFICATION_READY) {
		get_viewport()->set_embedding_subwindows(false);
		// get_tree()->set_debug_collisions_hint(true);
		// TODO: Locate lr2

		String lr2_dir = OS::get_singleton()->get_environment("LR2_PATH");
		bool found = DirAccess::exists(lr2_dir);

		if (found) {
			add_child(memnew(Whirled(CustomFS(lr2_dir))));
		} else {
			OS::get_singleton()->alert("Could not locate Lego Racers 2.");
			get_tree()->quit();
		}
	}
}

Init::Init() {
	Input::get_singleton()->set_use_accumulated_input(true);

	NavigationServer3D::get_singleton()->set_active(false);
	PhysicsServer3D::get_singleton()->set_active(false);
	PhysicsServer2D::get_singleton()->set_active(false);
	ScriptServer::set_scripting_enabled(false);
	TranslationServer::get_singleton()->set_enabled(false);

	DisplayServer::get_singleton()->window_set_min_size(Size2(1024, 600));
	DisplayServer::get_singleton()->screen_set_keep_on(false);
}