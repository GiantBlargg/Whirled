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
	uint32_t u1;
	uint32_t u2;
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

	int add(Ref<WRLEntry>, int index = -1);
	Ref<WRLEntry> remove(int index);

	void clear();
	Error load(FileAccess* file);
	Error save(FileAccess* file);

	class EventHandler {
	  protected:
		friend class WRL;
		virtual void _wrl_added(Ref<WRLEntry> entry, int index, bool synthetic = false){};
		virtual void _wrl_modified(Ref<WRLEntry> entry, int index, bool synthetic = false){};
		virtual void _wrl_removed(Ref<WRLEntry> entry, int index, bool synthetic = false){};
	};

	void add_event_handler(EventHandler* handler) { event_handlers.append(handler); }

  private:
	Vector<EventHandler*> event_handlers;
};
