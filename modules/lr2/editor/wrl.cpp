#include "wrl.h"

#include "../io/file_helper.h"

WRL::WRL() {}

const uint32_t WRL_MAGIC = 0x57324352;
const uint32_t WRL_VERSION = 0xb;
const uint32_t OBMG_MAGIC = 0x474d424f;

void WRL::clear() { entries.clear(); }

Error WRL::load(FileAccess* file) {
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

		Ref<WRLEntry> entry;

		{
			Ref<WRLUnknown> unknown(memnew(WRLUnknown));

			unknown->data.resize(next_chunk - file->get_position());
			file->get_buffer(unknown->data.ptrw(), unknown->data.size());

			entry = unknown;
		}

		entry->type = type;
		entry->u = u;
		entry->layer = layer;
		entry->name = name;

		entries.append(entry);
	}
	return OK;
}

Error WRL::save(FileAccess* file) {
	file->store_32(WRL_MAGIC);
	file->store_32(WRL_VERSION);

	uint64_t current_chunk = 8;
	for (int i = 0; i < entries.size(); i++) {
		Ref<WRLEntry> entry = entries.get(i);

		file->store_32(OBMG_MAGIC);
		store_string(file, entry->type, 24);
		file->store_32(entry->u);

		file->store_32(0);
		uint64_t length_start = file->get_position();

		file->store_32(entry->layer);
		store_string(file, entry->name, 24);

		{
			Ref<WRLUnknown> unknown = entry;
			file->store_buffer(unknown->data.ptr(), unknown->data.size());
		}

		uint64_t length = file->get_position() - length_start;
		file->seek(length_start - 4);
		file->store_32(length);
		file->seek(length_start + length);
	}

	return OK;
}