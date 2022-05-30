#include "wrl.hpp"

#include "../io/file_helper.h"
#include "core/templates/ordered_hash_map.h"

size_t WRL::Format::find_property(const String& name) const {
	for (int i = 0; i < properties.size(); i++) {
		if (name == properties[i].name)
			return i;
	}
	throw std::out_of_range("");
}

const Variant& WRL::Entry::get(const String& name) const {
	auto i = format.find_property(name);
	return properties[i];
}

void WRL::Entry::set(const String& name, const Variant& value) {
	auto i = format.find_property(name);
	const WRL::Format::Property& prop = format.properties[i];
	if (value.get_type() != prop.type)
		throw std::invalid_argument("");
	properties.set(i, value);
}

int WRL::get_index(String name) const {
	for (int i = 0; i < scene.size(); i++) {
		if (entries[scene[i].id].get<String>("name") == name) {
			return i;
		}
	}
	return -1;
}

const WRL::Format& WRL::get_entry_format(EntryID id) const { return entries[id.id].format; }
Variant WRL::get_entry_property(EntryID id, String prop_name) const { return entries[id.id].get(prop_name); }

void WRL::submit_change(const Change& change, String action_name) {
	for (auto prop = change.propertyChanges.front(); prop; prop = prop.next()) {
		entries.write[prop.key().first.id].set(prop.key().second, prop.value());
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
	entries.clear();
}

const uint32_t WRL_MAGIC = 0x57324352;
const uint32_t WRL_VERSION = 0xb;
const uint32_t OBMG_MAGIC = 0x474d424f;

Error WRL::load(Ref<FileAccess> file) {
	const OrderedHashMap<String, Format>& load_types = get_formats();

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

		Format format;
		if (load_types.has(type) && load_types[type].type == type && load_types[type].u == u) {
			format = load_types[type];
		} else {
			format = Format{type, u};
			format.properties.append_array(common_properties);
			format.properties.push_back({Variant::PACKED_BYTE_ARRAY, "data", length - 28});
		}

		Entry entry{.format = format};
		entry.properties.resize(format.properties.size());

		for (auto prop : format.properties) {
			if (prop.type == Variant::INT) {
				entry.set(prop.name, file->get_32());
			}

			else if (prop.type == Variant::FLOAT) {
				entry.set(prop.name, file->get_float());
			}

			else if (prop.type == Variant::STRING) {
				entry.set(prop.name, get_string(file, prop.length));
			}

			else if (prop.type == Variant::VECTOR2) {
				entry.set(prop.name, get_vector2(file));
			}

			else if (prop.type == Variant::VECTOR3) {
				entry.set(prop.name, get_vector3(file));
			}

			else if (prop.type == Variant::QUATERNION) {
				entry.set(prop.name, get_quaternion(file));
			}

			else if (prop.type == Variant::PACKED_BYTE_ARRAY) {
				Vector<uint8_t> data;
				data.resize(prop.length);
				file->get_buffer(data.ptrw(), data.size());
				entry.set(prop.name, data);
			}

			else {
				return Error::ERR_FILE_UNRECOGNIZED;
			}
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

Error WRL::save(Ref<FileAccess> file) {
	file->store_32(WRL_MAGIC);
	file->store_32(WRL_VERSION);

	for (auto i : scene) {
		const Entry& entry = entries[i.id];
		file->store_32(OBMG_MAGIC);
		store_string(file, entry.format.type, 24);
		file->store_32(entry.format.u);

		file->store_32(0); // Placeholder for length
		uint64_t length_start = file->get_position();

		for (auto prop : entry.format.properties) {
			if (prop.type == Variant::INT) {
				file->store_32(entry.get(prop.name));
			}

			else if (prop.type == Variant::FLOAT) {
				file->store_float(entry.get(prop.name));
			}

			else if (prop.type == Variant::STRING) {
				store_string(file, entry.get(prop.name), prop.length);
			}

			else if (prop.type == Variant::VECTOR2) {
				store_vector2(file, entry.get(prop.name));
			}

			else if (prop.type == Variant::VECTOR3) {
				store_vector3(file, entry.get(prop.name));
			}

			else if (prop.type == Variant::QUATERNION) {
				store_quaternion(file, entry.get(prop.name));
			}

			else if (prop.type == Variant::PACKED_BYTE_ARRAY) {
				Vector<uint8_t> data = entry.get(prop.name);
				file->store_buffer(data.ptr(), prop.length);
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
		auto prop_list = get_entry_format(entry).properties;
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
