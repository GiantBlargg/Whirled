#include "custom_fs.hpp"

String FSResolve::resolve_path(const String& p_path) const {
	DirAccessRef dir_access(DirAccess::create_for_path(root));

	String path = p_path.simplify_path();
	if (!path.begins_with("/")) {
		path = "/" + path;
	}

	if (DirAccess::exists(root.plus_file(p_path)) || FileAccess::exists(root.plus_file(p_path)))
		return path;

	dir_access->change_dir(root);
	Vector<String> path_segs = path.replace_first("/", "").split("/");
	String ret_path = "/";
	String next = ".";
	for (int i = 0; i < path_segs.size(); i++) {
		dir_access->change_dir(next);
		String path_seg = path_segs[i].to_lower();
		dir_access->list_dir_begin();
		do {
			next = dir_access->get_next();
			if (next == "")
				return path;
		} while (next.to_lower() != path_seg);
		dir_access->list_dir_end();
		ret_path = ret_path.plus_file(next);
	}

	return ret_path;
}

String FSResolve::map_path(const String& p_path) const { return root.plus_file(p_path); }

class LR2DirAccess : public DirAccess {
  private:
	const FSResolve fs_resolve;
	DirAccessRef dir_access;
	String current_dir;

	String _resolve_path(String p_path) {
		if (p_path.is_relative_path()) {
			p_path = current_dir.plus_file(p_path);
		}
		p_path = p_path.simplify_path();
		return fs_resolve.resolve_path(p_path);
	}

  public:
	LR2DirAccess(const FSResolve& p_fs_resolve)
		: fs_resolve(p_fs_resolve), dir_access(DirAccess::open(fs_resolve.root)) {}

	Error list_dir_begin() override { return dir_access->list_dir_begin(); }
	String get_next() override { return dir_access->get_next(); }
	bool current_is_dir() const override { return dir_access.f->current_is_dir(); }
	bool current_is_hidden() const override { return dir_access.f->current_is_hidden(); }
	void list_dir_end() override { dir_access->list_dir_end(); }

	int get_drive_count() override { return 0; }
	String get_drive(int p_drive) override { return ""; }

	Error change_dir(String p_dir) override {
		String try_path = _resolve_path(p_dir);
		Error err = dir_access->change_dir(fs_resolve.map_path(try_path));
		if (err == Error::OK)
			current_dir = try_path;
		return err;
	}
	String get_current_dir(bool p_include_drive = true) override { return current_dir; }
	Error make_dir(String p_dir) override {
		ERR_PRINT("NOT IMPLEMENTED");
		return dir_access->make_dir(p_dir);
	}

	bool file_exists(String p_file) override {
		return dir_access->file_exists(fs_resolve.map_path(_resolve_path(p_file)));
	}
	bool dir_exists(String p_dir) override { return dir_access->dir_exists(fs_resolve.map_path(_resolve_path(p_dir))); }
	uint64_t get_space_left() override {
		ERR_PRINT("NOT IMPLEMENTED");
		return dir_access->get_space_left();
	}

	Error rename(String p_from, String p_to) override {
		ERR_PRINT("NOT IMPLEMENTED");
		return dir_access->rename(p_from, p_to);
	}
	Error remove(String p_name) override {
		ERR_PRINT("NOT IMPLEMENTED");
		return dir_access->remove(p_name);
	}

	bool is_link(String p_file) override {
		ERR_PRINT("NOT IMPLEMENTED");
		return dir_access->is_link(p_file);
	}
	String read_link(String p_file) override {
		ERR_PRINT("NOT IMPLEMENTED");
		return dir_access->read_link(p_file);
	}
	Error create_link(String p_source, String p_target) override {
		ERR_PRINT("NOT IMPLEMENTED");
		return dir_access->create_link(p_source, p_target);
	}

	String get_filesystem_type() const override {
		ERR_PRINT("NOT IMPLEMENTED");
		return dir_access.f->get_filesystem_type();
	}
};

class LR2FileAccess : public FileAccess {
  private:
	const FSResolve fs_resolve;
	FileAccessRef file;

  public:
	LR2FileAccess(const FSResolve& p_fs_resolve)
		: fs_resolve(p_fs_resolve), file(FileAccess::create(FileAccess::ACCESS_FILESYSTEM)) {}

  public:
	uint32_t _get_unix_permissions(const String& p_file) {
		ERR_PRINT("NOT IMPLEMENTED");
		return file->_get_unix_permissions(p_file);
	}
	Error _set_unix_permissions(const String& p_file, uint32_t p_permissions) {
		ERR_PRINT("NOT IMPLEMENTED");
		return file->_set_unix_permissions(p_file, p_permissions);
	}

  protected:
	Error _open(const String& p_path, int p_mode_flags) {
		return file->reopen(fs_resolve.map_path(fs_resolve.resolve_path(p_path)), p_mode_flags);
	}
	uint64_t _get_modified_time(const String& p_file) {
		ERR_PRINT("NOT IMPLEMENTED");
		return file->get_modified_time(p_file);
	}

  public:
	void close() { file->close(); }
	bool is_open() const {
		ERR_PRINT("NOT IMPLEMENTED");
		return file.f->is_open();
	}

	String get_path() const {
		ERR_PRINT("NOT IMPLEMENTED");
		return file.f->get_path();
	}
	String get_path_absolute() const {
		ERR_PRINT("NOT IMPLEMENTED");
		return file.f->get_path_absolute();
	}

	void seek(uint64_t p_position) { file->seek(p_position); }
	void seek_end(int64_t p_position = 0) {
		ERR_PRINT("NOT IMPLEMENTED");
		file->seek_end();
	}
	uint64_t get_position() const { return file.f->get_position(); }
	uint64_t get_length() const { return file.f->get_length(); }

	bool eof_reached() const { return file.f->eof_reached(); }

	uint8_t get_8() const { return file.f->get_8(); }

	Error get_error() const {
		ERR_PRINT("NOT IMPLEMENTED");
		return file.f->get_error();
	}

	void flush() {
		ERR_PRINT("NOT IMPLEMENTED");
		file->flush();
	}
	void store_8(uint8_t p_dest) { file->store_8(p_dest); }

	bool file_exists(const String& p_name) {
		ERR_PRINT("NOT IMPLEMENTED");
		return file->file_exists(p_name);
	}
};

FileAccess* CustomFS::FileAccess_create() const { return memnew(LR2FileAccess(fs_resolve)); }
DirAccess* CustomFS::DirAccess_create() const { return memnew(LR2DirAccess(fs_resolve)); }

FileAccess* CustomFS::FileAccess_open(const String& p_path, int p_mode_flags, Error* r_error) const {
	FileAccess* ret = FileAccess_create();
	Error err = ret->reopen(p_path, p_mode_flags);

	if (r_error) {
		*r_error = err;
	}
	if (err != OK) {
		memdelete(ret);
		ret = nullptr;
	}

	return ret;
}

DirAccess* CustomFS::DirAccess_open(const String& p_path, Error* r_error) const {
	DirAccess* da = DirAccess_create();

	ERR_FAIL_COND_V_MSG(!da, nullptr, "Cannot create DirAccess for path '" + p_path + "'.");
	Error err = da->change_dir(p_path);
	if (r_error) {
		*r_error = err;
	}
	if (err != OK) {
		memdelete(da);
		return nullptr;
	}

	return da;
}

String CustomFS::canon_path(const String& p_path) const { return fs_resolve.resolve_path(p_path); }

bool CustomFS::file_exists(const String& p_path) const {
	DirAccessRef f = DirAccess_create();
	return f->file_exists(p_path);
}
bool CustomFS::dir_exists(const String& p_path) const {
	DirAccessRef f = DirAccess_create();
	return f->dir_exists(p_path);
}
bool CustomFS::exists(const String& p_path) const { return file_exists(p_path) || dir_exists(p_path); }

Vector<uint8_t> CustomFS::get_file_as_array(const String& p_path, Error* r_error) const {
	FileAccess* f = FileAccess_open(p_path, FileAccess::READ, r_error);
	if (!f) {
		if (r_error) { // if error requested, do not throw error
			return Vector<uint8_t>();
		}
		ERR_FAIL_V_MSG(Vector<uint8_t>(), "Can't open file from path '" + String(p_path) + "'.");
	}
	Vector<uint8_t> data;
	data.resize(f->get_length());
	f->get_buffer(data.ptrw(), data.size());
	memdelete(f);
	return data;
}
String CustomFS::get_file_as_string(const String& p_path, Error* r_error) const {
	Error err;
	Vector<uint8_t> array = get_file_as_array(p_path, &err);
	if (r_error) {
		*r_error = err;
	}
	if (err != OK) {
		if (r_error) {
			return String();
		}
		ERR_FAIL_V_MSG(String(), "Can't get file as string from path '" + String(p_path) + "'.");
	}

	String ret;
	ret.parse_utf8((const char*)array.ptr(), array.size());
	return ret;
}