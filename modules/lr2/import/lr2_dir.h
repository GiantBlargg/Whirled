#pragma once

#include "core/io/config_file.h"

class LR2Dir : public Object {
	GDCLASS(LR2Dir, Object);

  private:
	static LR2Dir* singleton;

  public:
	static LR2Dir* get_singleton() { return LR2Dir::singleton; }
	LR2Dir() { LR2Dir::singleton = this; }

  protected:
	static void _bind_methods() {
		ClassDB::bind_method("get_path", &LR2Dir::get_path);
		ClassDB::bind_method("set_path", &LR2Dir::set_path);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "path"), "set_path", "get_path");

		ClassDB::bind_method("resolve_path", &LR2Dir::resolve_path);

		ClassDB::bind_method("deduce_path", &LR2Dir::deduce_path);
	}

  private:
	const String configPath = "user://settings.cfg", section = "", key = "GamePath";

	Ref<ConfigFile> config;

  public:
	void init();

	String get_path();
	void set_path(String path);

	String resolve_path(String path);

	void deduce_path(String path);
};