#include "wrl.h"

#include "../io/file_helper.h"

WRL::WRL() {}

const uint32_t WRL_MAGIC = 0x57324352;
const uint32_t WRL_VERSION = 0xb;
const uint32_t OBMG_MAGIC = 0x474d424f;

void WRL::clear() {
	for (int i = 0; i < entries.size(); i++) {
		auto entry = entries[i];
		event_handler->_wrl_event(WRL::Removed, entry->name, entry);
	}
	entries.clear();
}

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

		uint64_t length = file->get_32();
		next_chunk = file->get_position() + length;

		uint32_t layer = file->get_32();
		String name = get_string(file, 24);
		String binding = get_string(file, 24);

		Ref<WRLEntry> entry;

		if (type == "cGeneralStatic" || type == "cGoldenBrick") {
			Ref<WRLGeneralStatic> e(memnew(WRLGeneralStatic));

			e->position = get_vector3(file);
			e->rotation = get_quaternion(file);
			e->u1 = file->get_32();
			e->u2 = file->get_32();
			e->collision_sound = file->get_32();
			e->model = get_string(file, 0x80);

			entry = e;

		} else {
			Ref<WRLUnknown> e(memnew(WRLUnknown));

			e->data.resize(next_chunk - file->get_position());
			file->get_buffer(e->data.ptrw(), e->data.size());

			entry = e;
		}

		entry->type = type;
		entry->u = u;
		entry->layer = layer;
		entry->name = name;
		entry->binding = binding;

		entries.append(entry);

		event_handler->_wrl_event(WRLEvent::Added, entry->name, entry);
	}
	return OK;
}

Error WRL::save(FileAccess* file) {
	file->store_32(WRL_MAGIC);
	file->store_32(WRL_VERSION);

	for (int i = 0; i < entries.size(); i++) {
		Ref<WRLEntry> entry = entries.get(i);

		file->store_32(OBMG_MAGIC);
		store_string(file, entry->type, 24);
		file->store_32(entry->u);

		file->store_32(0); // Placeholder for length
		uint64_t length_start = file->get_position();

		file->store_32(entry->layer);
		store_string(file, entry->name, 24);
		store_string(file, entry->binding, 24);

		Ref<WRLGeneralStatic> gs = entry;
		if (gs.is_valid()) {
			store_vector3(file, gs->position);
			store_quaternion(file, gs->rotation);
			file->store_32(gs->u1);
			file->store_32(gs->u2);
			file->store_32(gs->collision_sound);
			store_string(file, gs->model, 0x80);
		}
		Ref<WRLUnknown> u = entry;
		if (u.is_valid()) {
			file->store_buffer(u->data.ptr(), u->data.size());
		}

		uint64_t length = file->get_position() - length_start;
		file->seek(length_start - 4);
		file->store_32(length);
		file->seek(length_start + length);
	}

	return OK;
}