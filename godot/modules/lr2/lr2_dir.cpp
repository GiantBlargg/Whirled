#include "lr2_dir.h"

#include "core/os/dir_access.h"

LR2Dir* LR2Dir::singleton = NULL;

void LR2Dir::init() {
	config.instance();
	config->load(configPath);
}

String LR2Dir::get_path() {
	return config->get_value(section, key, "");
}
void LR2Dir::set_path(String path) {
	config->set_value(section, key, path);
	config->save(configPath);
}

String LR2Dir::resolve_path(String path) {
	DirAccess* current_dir = DirAccess::open(get_path());

	String next = "";

	auto path_split = path.replace("\\", "/").split("/");
	for (int i = 0; i < path_split.size(); i++) {
		auto path_dir = path_split[i];

		current_dir->list_dir_begin();
		do {
			next = current_dir->get_next();
			if(next == "") {
				return "";
			}
		} while (next.to_lower() != path_dir.to_lower());
		current_dir->list_dir_end();
		current_dir->change_dir(next);
	}
	return current_dir->get_current_dir() + "/" + next;
}

void LR2Dir::deduce_path(String path) {
	if (get_path() == "") {
		DirAccess* dir = DirAccess::open(path);
	}
}