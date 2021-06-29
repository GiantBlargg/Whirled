#pragma once

#include "core/io/dir_access.h"
#include "core/io/file_access.h"

class RemapFSAccess {
  private:
	static String path;
	String _path;

  public:
	static String get_path() { return RemapFSAccess::path; }
	static void set_path(String path) { RemapFSAccess::path = path.simplify_path(); }

	RemapFSAccess() { _path = RemapFSAccess::path; }
	String map_path(String path);
	String unmap_path(String path);
	String get() { return _path; }
};

class RemapDirAccess : public DirAccess {

  private:
	DirAccess* dir;
	RemapFSAccess map;

  public:
	RemapDirAccess();

	Error list_dir_begin() { return dir->list_dir_begin(); }
	String get_next() { return dir->get_next(); }
	bool current_is_dir() const { return dir->current_is_dir(); }
	bool current_is_hidden() const { return dir->current_is_hidden(); }
	void list_dir_end() { dir->list_dir_end(); }

	int get_drive_count() { return 0; }
	String get_drive(int p_drive) { return ""; }

	Error change_dir(String p_dir) { return dir->change_dir(map.map_path(p_dir)); }
	String get_current_dir(bool p_include_drive = true) { return map.unmap_path(dir->get_current_dir(true)); }
	Error make_dir(String p_dir) {
		ERR_PRINT("NOT IMPLEMENTED");
		return dir->make_dir(p_dir);
	}

	bool file_exists(String p_file) { return dir->file_exists(map.map_path(p_file)); }
	bool dir_exists(String p_dir) {
		ERR_PRINT("NOT IMPLEMENTED");
		return dir->dir_exists(p_dir);
	}
	uint64_t get_space_left() {
		ERR_PRINT("NOT IMPLEMENTED");
		return dir->get_space_left();
	}

	Error rename(String p_from, String p_to) {
		ERR_PRINT("NOT IMPLEMENTED");
		return dir->rename(p_from, p_to);
	}
	Error remove(String p_name) {
		ERR_PRINT("NOT IMPLEMENTED");
		return dir->remove(p_name);
	}

	bool is_link(String p_file) {
		ERR_PRINT("NOT IMPLEMENTED");
		return dir->is_link(p_file);
	}
	String read_link(String p_file) {
		ERR_PRINT("NOT IMPLEMENTED");
		return dir->read_link(p_file);
	}
	Error create_link(String p_source, String p_target) {
		ERR_PRINT("NOT IMPLEMENTED");
		return dir->create_link(p_source, p_target);
	}

	String get_filesystem_type() const {
		ERR_PRINT("NOT IMPLEMENTED");
		return dir->get_filesystem_type();
	}
};

class RemapFileAccess : public FileAccess {
  private:
	FileAccess* file;
	RemapFSAccess map;

  public:
	RemapFileAccess();

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
	Error _open(const String& p_path, int p_mode_flags) { return file->reopen(map.map_path(p_path), p_mode_flags); }
	uint64_t _get_modified_time(const String& p_file) {
		ERR_PRINT("NOT IMPLEMENTED");
		return file->get_modified_time(p_file);
	}

  public:
	void close() { file->close(); }
	bool is_open() const {
		ERR_PRINT("NOT IMPLEMENTED");
		return file->is_open();
	}

	String get_path() const {
		ERR_PRINT("NOT IMPLEMENTED");
		return file->get_path();
	}
	String get_path_absolute() const {
		ERR_PRINT("NOT IMPLEMENTED");
		return file->get_path_absolute();
	}

	void seek(uint64_t p_position) { file->seek(p_position); }
	void seek_end(int64_t p_position = 0) {
		ERR_PRINT("NOT IMPLEMENTED");
		file->seek_end();
	}
	uint64_t get_position() const { return file->get_position(); }
	uint64_t get_length() const { return file->get_length(); }

	bool eof_reached() const { return file->eof_reached(); }

	uint8_t get_8() const { return file->get_8(); }

	Error get_error() const {
		ERR_PRINT("NOT IMPLEMENTED");
		return file->get_error();
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