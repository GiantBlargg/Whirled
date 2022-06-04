#pragma once

#include "core/io/file_access.h"
#include "core/object/ref_counted.h"
#include "core/templates/hash_map.h"
#include "core/templates/pair.h"
#include "core/templates/vector.h"

class WRL : public RefCounted {
	GDCLASS(WRL, RefCounted);

  public:
	struct Format {
		String type;
		uint32_t u;

		struct Property {
			Variant::Type type;
			String name;
			uint32_t length;
			enum Flags { EntryID, File };
			Flags flags;
			String hint;
		};
		Vector<Property> properties;

		struct Model {
			String model;
			String position;
			String rotation;
			String scale;
			enum class Type { Prop, Terrain, Skybox };
			Type type = Type::Prop;
			Vector<String> uniforms;

			explicit operator bool() const {
				return !model.is_empty() || !position.is_empty() || !rotation.is_empty() || !scale.is_empty();
			}
		};
		Model model;

		size_t find_property(const String& name) const;
	};

	struct EntryID {
		int id = -1;
		inline friend bool operator==(const EntryID& lhs, const EntryID& rhs) { return lhs.id == rhs.id; }
		inline friend auto operator<=>(const EntryID& lhs, const EntryID& rhs) { return lhs.id <=> rhs.id; }
		explicit operator bool() const { return id != -1; }
	};

  private:
	static Vector<Format::Property> common_properties;
	static const HashMap<String, Format>& get_formats();

	struct Entry {
		Format format;
		Vector<Variant> properties;

		const Variant& get(const String& name) const;
		template <class T> T get(const String& name) const { return get(name); };
		void set(const String& name, const Variant& value);
	};
	Vector<Entry> entries;
	Vector<EntryID> scene;
	bool regen_scene_map = true;
	HashMap<int, EntryID> scene_map;

  public:
	Vector<EntryID> get_scene() { return scene.duplicate(); }
	HashMap<int, EntryID> get_scene_map() {
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

	const Format& get_entry_format(EntryID id) const;
	Variant get_entry_property(EntryID id, String prop_name) const;
	template <class T> T get_entry_property(EntryID id, String prop_name) const {
		return get_entry_property(id, prop_name);
	}

	struct Change {
		struct Hasher {
			static _FORCE_INLINE_ uint32_t hash(const Pair<EntryID, String>& key) {
				return ((key.first.id) + (key.second)).hash();
			}
		};
		typedef HashMap<Pair<EntryID, String>, Variant, Hasher> PropertyMap;
		PropertyMap propertyChanges;
		HashMap<int, EntryID> added;
		HashMap<int, EntryID> removed;
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