#pragma once

#include "core/io/file_access.h"
#include "core/object/ref_counted.h"

class WRLEntry : public RefCounted {
	GDCLASS(WRLEntry, RefCounted);

  public:
	String type;
	uint32_t u;
	uint32_t layer;
	String name;
	String binding;
};

class WRLGeneralStatic : public WRLEntry {
	GDCLASS(WRLGeneralStatic, WRLEntry);

  public:
	Vector3 position;
	Quaternion rotation;
	uint32_t collision_sound;
	String model;
};

class WRLUnknown : public WRLEntry {
	GDCLASS(WRLUnknown, WRLEntry);

  public:
	Vector<uint8_t> data;
};

class WRL : public RefCounted {
	GDCLASS(WRL, RefCounted);

  private:
	Vector<Ref<WRLEntry>> entries;

  public:
	WRL();

	void clear();

	Error load(FileAccess* file);
	Error save(FileAccess* file);
};
