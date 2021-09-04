#include "wrl.h"

#include "../io/file_helper.h"

#define EVENT(event)                                                                                                   \
	for (int i = 0; i < event_handlers.size(); i++) {                                                                  \
		event_handlers[i]->_wrl_##event;                                                                               \
	}

WRL::WRL() {}

int WRL::add(Ref<WRLEntry> entry, int index) {
	if (index == -1) {
		index = entries.size();
	}
	entries.insert(index, entry);
	EVENT(added(entry, index))
	return index;
}

Ref<WRLEntry> WRL::remove(int index) {
	if (selected == index)
		select(-1);
	Ref<WRLEntry> e = entries.get(index);
	entries.remove(index);
	EVENT(removed(e, index))
	return e;
}

void WRL::clear() {
	select(-1);
	Vector<Ref<WRLEntry>> e = entries;
	entries.clear();
	EVENT(cleared(e))
}

const uint32_t WRL_MAGIC = 0x57324352;
const uint32_t WRL_VERSION = 0xb;
const uint32_t OBMG_MAGIC = 0x474d424f;

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

		} else if (type == "cSkyBox") {
			Ref<WRLSkyBox> e(memnew(WRLSkyBox));

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

		add(entry);
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

void WRL::select(int index) {
	if (selected == index)
		return;
	if (selected > -1)
		EVENT(deselected(entries[selected], selected))
	selected = index;
	if (selected > -1)
		EVENT(selected(entries[selected], selected))
}

void WRL::select(String name) {
	for (int i = 0; i < entries.size(); i++) {
		if (name == entries[i]->name) {
			return select(i);
		}
	}
	ERR_PRINT("Could not find " + name + " for selection.");
}