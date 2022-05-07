#pragma once

#include "core/io/file_access.h"
#include "core/object/ref_counted.h"
#include "core/templates/ordered_hash_map.h"
#include "core/templates/pair.h"
#include "core/templates/vector.h"

struct Entry;

class WRL : public RefCounted {
	GDCLASS(WRL, RefCounted);

  public:
	struct EntryID {
		int id = -1;
		inline friend bool operator==(const EntryID& lhs, const EntryID& rhs) { return lhs.id == rhs.id; }
		inline friend auto operator<=>(const EntryID& lhs, const EntryID& rhs) { return lhs.id <=> rhs.id; }
		explicit operator bool() const { return id != -1; }
	};

  private:
	Vector<Entry*> entries;
	Vector<EntryID> scene;
	bool regen_scene_map = true;
	OrderedHashMap<int, EntryID> scene_map;

  public:
	Vector<EntryID> get_scene() { return scene.duplicate(); }
	OrderedHashMap<int, EntryID> get_scene_map() {
		if (regen_scene_map) {
			scene_map.clear();
			for (int i = 0; i < scene.size(); i++) {
				scene_map.insert(i, scene[i]);
			}
			regen_scene_map = false;
		}
		return scene_map;
	}

	int get_index(String name) const;
	int get_index(EntryID id) const { return scene.find(id); }

	List<PropertyInfo> get_entry_property_list(EntryID id) const;
	Variant get_entry_property(EntryID id, String prop_name) const;

	struct Change {
		struct Hasher {
			static _FORCE_INLINE_ uint32_t hash(const Pair<EntryID, String>& key) {
				return ((key.first.id) + (key.second)).hash();
			}
		};
		typedef OrderedHashMap<Pair<EntryID, String>, Variant, Hasher> PropertyMap;
		PropertyMap propertyChanges;
		OrderedHashMap<int, EntryID> added;
		OrderedHashMap<int, EntryID> removed;
		bool select_changed;
		Pair<int, EntryID> select;
	};
	void submit_change(const Change&, String action_name = "");

	void select(EntryID);
	void select(int index) { select(scene.get(index)); }

	void clear();
	Error load(Ref<FileAccess> file);
	Error save(Ref<FileAccess> file);

  public:
	class EventHandler {
	  protected:
		friend class WRL;
		Ref<WRL> wrl;

		virtual void _wrl_changed(const Change&, bool reset = false) = 0;
		virtual bool lite_init() { return false; }

	  public:
		void wrl_connect(Ref<WRL> p_wrl) {
			if (wrl == p_wrl)
				return;

			Change change;
			if (!lite_init()) {
				if (wrl.is_valid()) {
					change.removed = wrl->get_scene_map();
					wrl->event_handlers.erase(this);
				};
			}
			wrl = p_wrl;
			if (!lite_init()) {
				if (wrl.is_valid()) {
					wrl->event_handlers.append(this);
					change.added = wrl->get_scene_map();
					wrl->fixup_change(change);
				}
			}
			this->_wrl_changed(change, true);
		}
		EventHandler() = default;
		EventHandler(const EventHandler&) = delete;
		EventHandler(EventHandler&&) = delete;
		EventHandler& operator=(const EventHandler&) = delete;
		EventHandler& operator=(EventHandler&&) = delete;

		~EventHandler() {
			if (wrl.is_valid())
				wrl->event_handlers.erase(this);
		}
	};

  private:
	void fixup_change(Change&);
	void emit_change(Change change, bool reset = false);
	Vector<EventHandler*> event_handlers;
};