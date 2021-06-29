#include "init.h"

#include "core/string/translation.h"
#include "editor/whirled.h"
#include "io/remap_fs_access.h"
#include "servers/navigation_server_3d.h"

void Init::_notification(int p_notification) {
	if (p_notification == NOTIFICATION_READY) {

		// TODO: Locate lr2

		String lr2_dir = OS::get_singleton()->get_environment("LR2_PATH");
		bool found = DirAccess::exists(lr2_dir);

		if (found) {
			RemapFSAccess::set_path(lr2_dir);
			DirAccess::make_default<RemapDirAccess>(DirAccess::ACCESS_USERDATA);
			FileAccess::make_default<RemapFileAccess>(FileAccess::ACCESS_USERDATA);

			get_parent()->call_deferred("add_child", memnew(Whirled));
			queue_delete();
		} else {
			DisplayServer::get_singleton()->alert("Could not locate Lego Racers 2.");
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