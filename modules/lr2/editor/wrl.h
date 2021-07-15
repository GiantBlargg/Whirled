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
	int selected = -1;

  public:
	WRL();

	int add(Ref<WRLEntry>, int index = -1);
	Ref<WRLEntry> remove(int index);

	void clear();
	Error load(FileAccess* file);
	Error save(FileAccess* file);

	void select(int index);
	void select(String name);

	class EventHandler {
	  protected:
		friend class WRL;
		virtual void _wrl_added(Ref<WRLEntry> entry, int index){};
		virtual void _wrl_modified(Ref<WRLEntry> entry, int index){};
		virtual void _wrl_removed(Ref<WRLEntry> entry, int index){};
		virtual void _wrl__added(Ref<WRLEntry> entry, int index) {
			_wrl_added(entry, index);
			_wrl_modified(entry, index);
		};
		virtual void _wrl_cleared(Vector<Ref<WRLEntry>> entries) {
			for (int i = entries.size() - 1; i >= 0; i--) {
				_wrl_removed(entries[i], i);
			}
		};

		virtual void _wrl_selected(Ref<WRLEntry> entry, int index) {}
		virtual void _wrl_deselected(Ref<WRLEntry> entry, int index) {}
	};

	void add_event_handler(EventHandler* handler) { event_handlers.append(handler); }

  private:
	Vector<EventHandler*> event_handlers;
};
