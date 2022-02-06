#pragma once

#include "core/io/file_access.h"
#include "core/object/ref_counted.h"
#include "core/templates/vector.h"

struct Entry;

class WRL : public RefCounted {
	GDCLASS(WRL, RefCounted);

  public:
	struct EntryID {
		int id = -1;
	};

  private:
	Vector<Entry*> entries;
	Vector<EntryID> scene;

  public:
	Vector<EntryID> get_scene() { return scene.duplicate(); }

	int get_index(String name);
	int get_index(EntryID id);

	void select(EntryID);
	void select(int index) { select(scene.get(index)); }

	void clear();
	Error load(FileAccess* file);

  public:
#include "define_enums.gen.ipp"
	struct Event {
		enum class Type { Cleared, Inited, Added, Removed, Modified, Selected, Deselected };

		Type event_type;
		EntryType entry_type;
		EntryID id;
		int index;
		Field field;
	};
	class EventHandler {
	  protected:
		friend class WRL;
		Ref<WRL> wrl;
		virtual void _wrl_event(const Event&) = 0;

		void _wrl_emit_add_all() {
			auto scene = wrl->get_scene();
			for (int i = 0; i < scene.size(); i++) {
				auto id = scene[i];
				_wrl_event(Event{
					.event_type = Event::Type::Added,
					.entry_type = wrl->get_Entry_EntryType(id),
					.id = id,
					.index = i});
			}
		}

		void _wrl_emit_remove_all() {
			auto scene = wrl->get_scene();
			for (int i = scene.size() - 1; i >= 0; i--) {
				auto id = scene[i];
				_wrl_event(Event{
					.event_type = Event::Type::Removed,
					.entry_type = wrl->get_Entry_EntryType(id),
					.id = id,
					.index = i});
			}
		}

#include "../wrl/emit_all_modified.gen.ipp"

	  public:
		void wrl_connect(Ref<WRL> p_wrl) {
			wrl = p_wrl;
			wrl->event_handlers.append(this);
			// this->_wrl_event(Event{.event_type = Event::Type::Inited});
		}
	};

  private:
	void emit_event(const Event& event) {
		for (int i = 0; i < event_handlers.size(); i++) {
			event_handlers[i]->_wrl_event(event);
		}
	}
	Vector<EventHandler*> event_handlers;

  public:
#include "declare_getters.gen.ipp"
#include "get_entry_type.gen.ipp"
  private:
#include "declare_internal_setters.gen.ipp"
  public:
#include "declare_setters.gen.ipp"
};