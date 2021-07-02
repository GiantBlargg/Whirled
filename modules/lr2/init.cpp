#include "init.h"

#include "core/string/translation.h"
#include "editor/whirled.h"
#include "io/remap_fs_access.h"
#include "servers/navigation_server_3d.h"

void Init::_notification(int p_notification) {
	if (p_notification == NOTIFICATION_READY) {
		DisplayServer::get_singleton()->delete_sub_window(window);

		// TODO: Locate lr2

		String lr2_dir = OS::get_singleton()->get_environment("LR2_PATH");
		bool found = DirAccess::exists(lr2_dir);

		if (found) {
			RemapFSAccess::set_path(lr2_dir);
			DirAccess::make_default<RemapDirAccess<RemapFSAccess, DefaultDirAccess<DirAccess::ACCESS_FILESYSTEM>>>(
				DirAccess::ACCESS_RESOURCES);
			FileAccess::make_default<CaseInsensitiveFileAccess<
				New<RemapFileAccess<RemapFSAccess, DefaultFileAccess<FileAccess::ACCESS_FILESYSTEM>>>,
				DefaultDirAccess<DirAccess::ACCESS_RESOURCES>>>(FileAccess::ACCESS_RESOURCES);

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

	{
		auto DS = DisplayServer::get_singleton();

		DS->window_set_min_size(Size2(1024, 600));
		DS->screen_set_keep_on(false);

		window = DS->create_sub_window(DisplayServer::WINDOW_MODE_WINDOWED, 0);
		DS->show_window(window);
	}
}