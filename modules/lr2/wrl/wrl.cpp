#include "wrl.hpp"

#include "../io/file_helper.h"
#include "core/templates/ordered_hash_map.h"

#include "wrl_structs.hpp"

int WRL::get_index(String name) const {
	for (int i = 0; i < scene.size(); i++) {
		if (entries[scene[i].id]->name == name) {
			return i;
		}
	}
	return -1;
}

List<PropertyInfo> WRL::get_entry_property_list(EntryID id) const {
	List<PropertyInfo> property_info;
	entries[id.id]->get_property_list(&property_info);
	List<PropertyInfo> ret;
	for (auto prop : property_info) {
		if (prop.hint_string == "Script")
			continue;
		if (prop.type == Variant::Type::NIL)
			continue;

		ret.push_back(prop);
	}
	return ret;
}
Variant WRL::get_entry_property(EntryID id, String prop_name) const {
	if (entries.size() > id.id)
		return entries[id.id]->get(prop_name);
	return Variant();
}
void WRL::set_entry_property(EntryID id, String prop_name, Variant value, bool) {
	Change::PropertyMap propertyChanges;
	propertyChanges.insert({id, prop_name}, value);
	submit_change(Change{.propertyChanges = propertyChanges});
}

void WRL::submit_change(const Change& change, String action_name) {
	for (auto prop = change.propertyChanges.front(); prop; prop = prop.next()) {
		entries[prop.key().first.id]->set(prop.key().second, prop.value());
	}
	emit_change(change);
}

void WRL::select(EntryID id) {
	int index = get_index(id);
	emit_change(Change{.select_changed = true, .select = {index, id}});
}

void WRL::clear() {
	emit_change(Change{.removed = get_scene_map(), .select_changed = true, .select = {-1, EntryID()}}, true);
	scene.clear();
	regen_scene_map = true;
	for (auto& e : entries) {
		memdelete(e);
	}
	entries.clear();
}

const uint32_t WRL_MAGIC = 0x57324352;
const uint32_t WRL_VERSION = 0xb;
const uint32_t OBMG_MAGIC = 0x474d424f;

Error WRL::load(FileAccess* file) {
	static OrderedHashMap<String, StringName> load_types;
	if (load_types.is_empty()) {
		ClassDB::register_class<GeneralStatic>();
		load_types.insert("cGeneralStatic", "GeneralStatic");
		load_types.insert("cGoldenBrick", "GeneralStatic");
		ClassDB::register_class<GeneralMobile>();
		load_types.insert("cGeneralMobile", "GeneralMobile");
		load_types.insert("cBonusPickup", "GeneralMobile");
		ClassDB::register_class<LegoTerrain>();
		load_types.insert("cLegoTerrain", "LegoTerrain");
		ClassDB::register_class<SkyBox>();
		load_types.insert("cSkyBox", "SkyBox");
	}

	clear();

	ERR_FAIL_COND_V_MSG(file->get_32() != WRL_MAGIC, ERR_FILE_UNRECOGNIZED, "Not a WRL file.");
	ERR_FAIL_COND_V_MSG(file->get_32() != WRL_VERSION, ERR_FILE_UNRECOGNIZED, "Wrong WRL version");

	uint64_t next_chunk = 8;
	while (file->get_length() > next_chunk) {
		file->seek(next_chunk);
		ERR_FAIL_COND_V_MSG(file->get_32() != OBMG_MAGIC, ERR_FILE_CORRUPT, "Couldn't find OBMG header");

		String type = get_string(file, 24);
		uint32_t u = file->get_32();

		uint32_t length = file->get_32();
		next_chunk = file->get_position() + length;

		StringName class_type;
		if (load_types.has(type)) {
			class_type = load_types[type];
		}

		Entry* entry;

		if (class_type != "") {
			entry = static_cast<Entry*>(ClassDB::instantiate(class_type));

			entry->type = type;
			entry->u = u;

			List<PropertyInfo> property_info;
			entry->get_property_list(&property_info);
			for (auto prop : property_info) {
				if (prop.hint_string == "Script")
					continue;
				if (prop.type == Variant::NIL)
					continue;

				if (prop.name == "type")
					continue;
				if (prop.name == "u")
					continue;

				if (prop.type == Variant::INT) {
					entry->set(prop.name, file->get_32());
				}

				else if (prop.type == Variant::FLOAT) {
					entry->set(prop.name, file->get_float());
				}

				else if (prop.type == Variant::STRING) {
					if (prop.name == "name" || prop.hint == PROPERTY_HINT_NODE_PATH_VALID_TYPES) {
						entry->set(prop.name, get_string(file, 24));
					} else if (prop.hint == PROPERTY_HINT_FILE || prop.hint == PROPERTY_HINT_DIR) {
						entry->set(prop.name, get_string(file, 0x80));
					} else {
						print_error("Unknown String length");
					}
				}

				else if (prop.type == Variant::VECTOR2) {
					entry->set(prop.name, get_vector2(file));
				}

				else if (prop.type == Variant::VECTOR3) {
					entry->set(prop.name, get_vector3(file));
				}

				else if (prop.type == Variant::QUATERNION) {
					entry->set(prop.name, get_quaternion(file));
				}

				else if (prop.type == Variant::PACKED_BYTE_ARRAY) {
					Vector<uint8_t> data;
					data.resize(prop.hint_string.to_int());
					file->get_buffer(data.ptrw(), data.size());
					entry->set(prop.name, data);
				}

				else {
					return Error::ERR_FILE_UNRECOGNIZED;
				}
			}
		} else {
			Unknown* e = memnew(Unknown);

			e->type = type;
			e->u = u;
			e->layer = file->get_32();
			e->name = get_string(file, 24);
			e->binding = get_string(file, 24);

			e->data.resize(next_chunk - file->get_position());
			file->get_buffer(e->data.ptrw(), e->data.size());
			entry = e;
		}

		scene.append(EntryID{entries.size()});
		entries.append(entry);

		if (file->get_position() != next_chunk) {
			WARN_PRINT("Wrong amount of data read: " + type);
		}
	}
	regen_scene_map = true;
	emit_change(Change{.added = get_scene_map()}, true);
	return OK;
}

Error WRL::save(FileAccess* file) {
	file->store_32(WRL_MAGIC);
	file->store_32(WRL_VERSION);

	for (auto i : scene) {
		Entry* entry = entries[i.id];
		file->store_32(OBMG_MAGIC);
		store_string(file, entry->type, 24);
		file->store_32(entry->u);

		file->store_32(0); // Placeholder for length
		uint64_t length_start = file->get_position();

		List<PropertyInfo> property_info;
		entry->get_property_list(&property_info);
		for (auto prop : property_info) {
			if (prop.hint_string == "Script")
				continue;
			if (prop.type == Variant::NIL)
				continue;

			if (prop.name == "type")
				continue;
			if (prop.name == "u")
				continue;

			if (prop.type == Variant::INT) {
				file->store_32(entry->get(prop.name));
			}

			else if (prop.type == Variant::FLOAT) {
				file->store_float(entry->get(prop.name));
			}

			else if (prop.type == Variant::STRING) {
				if (prop.name == "name" || prop.hint == PROPERTY_HINT_NODE_PATH_VALID_TYPES) {
					store_string(file, entry->get(prop.name), 24);
				} else if (prop.hint == PROPERTY_HINT_FILE || prop.hint == PROPERTY_HINT_DIR) {
					store_string(file, entry->get(prop.name), 0x80);
				} else {
					print_error("Unknown String length");
				}
			}

			else if (prop.type == Variant::VECTOR2) {
				store_vector2(file, entry->get(prop.name));
			}

			else if (prop.type == Variant::VECTOR3) {
				store_vector3(file, entry->get(prop.name));
			}

			else if (prop.type == Variant::QUATERNION) {
				store_quaternion(file, entry->get(prop.name));
			}

			else if (prop.type == Variant::PACKED_BYTE_ARRAY) {
				Vector<uint8_t> data = entry->get(prop.name);
				file->store_buffer(data.ptr(), data.size());
			}

			else {
				return Error::ERR_FILE_UNRECOGNIZED;
			}
		}

		uint64_t length = file->get_position() - length_start;
		file->seek(length_start - 4);
		file->store_32(length);
		file->seek(length_start + length);
	}

	return OK;
}

void WRL::fixup_change(Change& change) {
	for (auto add = change.added.front(); add; add = add.next()) {
		EntryID entry = add.value();
		auto prop_list = get_entry_property_list(entry);
		for (auto prop : prop_list) {
			Pair<EntryID, String> change_key(entry, prop.name);
			if (!change.propertyChanges.has(change_key)) {
				change.propertyChanges.insert(change_key, get_entry_property(entry, prop.name));
			}
		}
	}
}

void WRL::emit_change(Change change, bool reset) {
	fixup_change(change);
	for (auto handler : event_handlers.duplicate()) {
		handler->_wrl_changed(change, reset);
	}
}
