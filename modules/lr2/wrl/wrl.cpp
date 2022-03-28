#include "wrl.hpp"

#include "../io/file_helper.h"

#include "define_structs.gen.ipp"

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
	entries.clear();
}

const uint32_t WRL_MAGIC = 0x57324352;
const uint32_t WRL_VERSION = 0xb;
const uint32_t OBMG_MAGIC = 0x474d424f;

#include "load_structs.gen.ipp"

Error WRL::load(FileAccess* file) {
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

		uint32_t layer = file->get_32();
		String name = get_string(file, 24);
		String binding = get_string(file, 24);

		Entry* entry = load_type(type, file, next_chunk);
		if (entry == nullptr) {
			// WARN_PRINT("Unknown chunk: " + type);
			Unknown* e = new Unknown();
			e->data.resize(next_chunk - file->get_position());
			file->get_buffer(e->data.ptrw(), e->data.size());
			entry = e;
		}

		entry->type = type;
		entry->u = u;
		entry->layer = layer;
		entry->name = name;
		entry->binding = binding;

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
