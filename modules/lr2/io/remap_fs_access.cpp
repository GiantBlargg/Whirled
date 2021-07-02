#include "remap_fs_access.h"

String RemapFSAccess::path = "";

String RemapFSAccess::map_path(String p_path) { return p_path.replace_first("res:/", _path); }
String RemapFSAccess::unmap_path(String p_path) {
	String path = p_path.replace_first(_path, "res:/");
	if (path == "res:/") {
		path = path + "/";
	}
	return path;
}
