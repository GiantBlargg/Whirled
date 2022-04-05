#pragma once

#include "core/io/dir_access.h"
#include "core/io/file_access.h"

class FSResolve {
  public:
	const String root;

	FSResolve(const String& p_root) : root(p_root) {}
	String resolve_path(const String& p_path) const;
	String map_path(const String& p_path) const;
};

class CustomFS {
  private:
	FSResolve fs_resolve;

  public:
	CustomFS(const String& p_root) : fs_resolve(p_root) {}

	FileAccess* FileAccess_create() const;
	FileAccess* FileAccess_open(const String& p_path, int p_mode_flags, Error* = nullptr) const;

	DirAccess* DirAccess_create() const;
	DirAccess* DirAccess_open(const String& p_path, Error* = nullptr) const;

	String canon_path(const String& p_path) const;

	bool file_exists(const String& p_path) const;
	bool dir_exists(const String& p_path) const;
	bool exists(const String& p_path) const;

	Vector<uint8_t> get_file_as_array(const String& p_path, Error* r_error = nullptr) const;
	String get_file_as_string(const String& p_path, Error* r_error = nullptr) const;
};