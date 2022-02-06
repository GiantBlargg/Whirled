#include "wrl.hpp"

#include "../io/file_helper.h"

#include "define_structs.gen.ipp"

int WRL::get_index(String name) {
	for (int i = 0; i < scene.size(); i++) {
		if (entries[scene[i].id]->name == name) {
			return i;
		}
	}
	return -1;
}
int WRL::get_index(EntryID id) {
	for (int i = 0; i < scene.size(); i++) {
		if (scene[i].id == id.id) {
			return i;
		}
	}
	return -1;
}

void WRL::select(EntryID id) {
	int index = get_index(id);
	emit_event(Event{
		.event_type = Event::Type::Selected,
		.entry_type = get_Entry_EntryType(id),
		.id = id,
		.index = index,
	});
}

void WRL::clear() {
	emit_event(Event{.event_type = Event::Type::Cleared});
	scene.clear();
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
	emit_event(Event{.event_type = Event::Type::Inited});
	return OK;
}

#include "define_getters.gen.ipp"
#include "define_setters.gen.ipp"