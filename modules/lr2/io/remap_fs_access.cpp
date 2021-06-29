#include "remap_fs_access.h"

String RemapFSAccess::path = "";

String RemapFSAccess::map_path(String p_path) { return p_path.replace_first("user:/", _path); }
String RemapFSAccess::unmap_path(String p_path) {
	String path = p_path.replace_first(_path, "user:/");
	if (path == "user:/") {
		path = path + "/";
	}
	return path;
}

RemapDirAccess::RemapDirAccess() {
	dir = DirAccess::create(ACCESS_FILESYSTEM);
	dir->change_dir(map.get());
};

RemapFileAccess::RemapFileAccess() { file = FileAccess::create(ACCESS_FILESYSTEM); }