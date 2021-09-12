#pragma once

#include "core/io/file_access.h"
#include "core/object/ref_counted.h"

class WRL : public RefCounted {
	GDCLASS(WRL, RefCounted);

  public:
	class Entry : public RefCounted {
		GDCLASS(Entry, RefCounted);

	  public:
		String type;
		uint32_t u;
		uint32_t layer;
		String name;
		String binding;
	};

	class GeneralStatic : public Entry {
		GDCLASS(GeneralStatic, Entry);

	  public:
		Vector3 position;
		Quaternion rotation;
		uint32_t u1;
		uint32_t u2;
		uint32_t collision_sound;
		String model;
	};

	class SkyBox : public Entry {
		GDCLASS(SkyBox, Entry);

	  public:
		String model;
	};

	class Unknown : public Entry {
		GDCLASS(Unknown, Entry);

	  public:
		Vector<uint8_t> data;
	};

  private:
	Vector<Ref<Entry>> entries;
	int selected = -1;

  public:
	WRL();

	int add(Ref<Entry>, int index = -1);
	Ref<Entry> remove(int index);

	void clear();
	Error load(FileAccess* file);
	Error save(FileAccess* file);

	void select(int index);
	void select(String name);

	class EventHandler {
	  protected:
		friend class WRL;
		virtual void _wrl_added(Ref<Entry> entry, int index){};
		virtual void _wrl_modified(Ref<Entry> entry, int index){};
		virtual void _wrl_removed(Ref<Entry> entry, int index){};
		virtual void _wrl_cleared(Vector<Ref<Entry>> entries) {
			for (int i = entries.size() - 1; i >= 0; i--) {
				_wrl_removed(entries[i], i);
			}
		};

		virtual void _wrl_selected(Ref<Entry> entry, int index) {}
		virtual void _wrl_deselected(Ref<Entry> entry, int index) {}
	};

	void add_event_handler(EventHandler* handler) { event_handlers.append(handler); }

  private:
	Vector<EventHandler*> event_handlers;
};
