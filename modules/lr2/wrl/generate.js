const data_types = {
	"uint8*": field => ({
		storage_type: "Vector<uint8_t>", load_func: (field_address, file, next_chunk) => [
			`${field_address}.resize(${next_chunk} - ${file}->get_position());`,
			`\t\tfile->get_buffer(${field_address}.ptrw(), ${field_address}.size());`
		].join("\n")
	}),
	"uint8[]": field => ({
		storage_type: "uint8_t", extern_type: "Vector<uint8_t>", postfix: `[${field.length}]`,
		load_func: (field_address, file) => `${file}->get_buffer(${field_address}, ${field.length})`,
		getter: field_address => [
			"\tVector<uint8_t> v;",
			`\tv.resize(${field.length});`,
			`\tmemcpy(v.ptrw(), ${field_address}, ${field.length});`,
			`\treturn v;`
		],
		setter: (field_address, value) => [`\tmemcpy(${field_address}, ${value}.ptrw(), ${field.length});`]
	}),
	"uint32": { storage_type: "uint32_t", load_func: (field_address, file) => `${field_address} = ${file}->get_32()`, widget: "IntWidget" },
	"float": { load_func: (field_address, file) => `${field_address} = ${file}->get_float()`, widget: "FloatWidget" },
	"String": field => ({ load_func: (field_address, file) => `${field_address} = get_string(${file}, ${field.length})`, widget: "StringWidget" }),
	"Vector2": { load_func: (field_address, file) => `${field_address} = get_vector2(${file})`, widget: "Vector2Widget" },
	"Vector3": { load_func: (field_address, file) => `${field_address} = get_vector3(${file})`, widget: "Vector3Widget" },
	"Quaternion": { load_func: (field_address, file) => `${field_address} = get_quaternion(${file})`, widget: "RotationWidget" }
};

const entry_types = {
	"Entry": {
		fields: [
			{ type: "String", name: "type", static: true, length: 24 },
			{ type: "uint32", name: "u" },
			{ type: "uint32", name: "layer" },
			{ type: "String", name: "name", length: 24 },
			{ type: "String", name: "binding", length: 24 },
		]
	},
	"GeneralStatic": {
		type: ["cGeneralStatic", "cGoldenBrick"],
		fields: [
			{ type: "Vector3", name: "position" },
			{ type: "Quaternion", name: "rotation" },
			{ type: "uint32", name: "u1" },
			{ type: "uint32", name: "u2" },
			{ type: "uint32", name: "collision_sound" },
			{ type: "String", name: "model", length: 0x80 },
		]
	},
	"GeneralMobile": {
		type: ["cGeneralMobile", "cBonusPickup"],
		fields: [
			{ type: "Vector3", name: "position" },
			{ type: "Quaternion", name: "rotation" },
			{ type: "uint32", name: "u1" },
			{ type: "uint32", name: "u2" },
			{ type: "uint32", name: "collision_sound" },
			{ type: "float", name: "weight" },
			{ type: "Vector3", name: "u3" },
			{ type: "String", name: "model", length: 0x80 },
			{ type: "uint32", name: "u4" },
		]
	},
	"LegoTerrain": {
		type: "cLegoTerrain",
		fields: [
			{ type: "Vector3", name: "position" },
			{ type: "Quaternion", name: "rotation" },
			{ type: "float", name: "u1" },
			{ type: "float", name: "u2" },
			{ type: "uint32", name: "u3" },
			{ type: "String", name: "model", length: 0x80 },
			{ type: "uint32", name: "u4" },
			{ type: "Vector3", name: "scale" },
			{ type: "uint8[]", name: "u5", length: 0x10 },
			{ type: "Vector2", name: "texture_scale" },
			{ type: "uint8[]", name: "u6", length: 0x60 },
		]
	},
	"SkyBox": {
		type: "cSkyBox",
		fields: [
			{ type: "String", name: "model", length: 0x80 },
		]
	},
	"Unknown": { type: "_", fields: [{ type: "uint8*", name: "data" }] }
}

function convertToArray(v) {
	if (v == undefined) return [];
	if (v instanceof Array) return v;
	return [v];
}

const convert_from_record = Object.entries(entry_types).map(([typename, entry], i, array) => ({ ...entry, typename: typename, type: convertToArray(entry.type), parent: i == 0 ? undefined : array[0][0] }));

function get_data_type(field) {
	let data_type = data_types[field.type];
	if (typeof data_type == "function") data_type = data_type(field);
	if (data_type == undefined)
		console.error(`Field Type ${field.type} not defiend`);
	if (data_type.storage_type == undefined) data_type.storage_type = field.type;
	if (data_type.extern_type == undefined) data_type.extern_type = data_type.storage_type;
	if (data_type.postfix == undefined) data_type.postfix = "";
	if (data_type.getter == undefined) data_type.getter = field => `\treturn ${field};`
	if (data_type.setter == undefined) data_type.setter = (field, value) => `\t${field} = ${value};`
	return data_type;
}
const apply_format_data = convert_from_record.map(entry => ({ ...entry, fields: entry.fields.map(field => ({ ...get_data_type(field), ...field })) }))

const mapped_fields = apply_format_data.flatMap((entry) => entry.fields.map(field => ({ ...field, typename: entry.typename })));
const mutable_fields = mapped_fields.filter(field => !field.static);

const widget_fields = mapped_fields.filter(field => field.widget != undefined);

const generated_includes = {
	"declare_getters": mapped_fields.map(field =>
		`${field.extern_type} get_${field.typename}_${field.name}(EntryID id);`
	),
	"define_getters": mapped_fields.map(field => [
		`${field.extern_type} WRL::get_${field.typename}_${field.name}(EntryID id) {`,
		field.getter(`static_cast<${field.typename}*>(entries[id.id])->${field.name}`),
		"}\n"]
	),
	"declare_setters": mutable_fields.map(field => [
		`void set_${field.typename}_${field.name}(EntryID id, ${field.extern_type} value, bool commit = true) {__no_history_set_${field.typename}_${field.name}(id, value);}`
	]),
	"declare_internal_setters": mutable_fields.map(field =>
		`void __no_history_set_${field.typename}_${field.name}(EntryID id, ${field.extern_type} value);`
	),
	"define_setters": mutable_fields.map(field => [
		`void WRL::__no_history_set_${field.typename}_${field.name}(EntryID id, ${field.extern_type} value) {`,
		field.setter(`static_cast<${field.typename}*>(entries[id.id])->${field.name}`, "value"),
		`\temit_event(WRL::Event{`,
		`\t\t.event_type = WRL::Event::Type::Modified,`,
		`\t\t.entry_type = get_Entry_EntryType(id),`,
		`\t\t.id = id,`,
		`\t\t.index = get_index(id),`,
		`\t\t.field = WRL::Field::${field.typename}_${field.name},`,
		`\t});`,
		"}\n"
	]),
	"define_structs": apply_format_data.map((entry) => [
		`struct ${entry.typename}${entry.parent == undefined ? "" : ` : public ${entry.parent}`} {`,
		entry.fields.map(field =>
			`\t${field.storage_type} ${field.name}${field.postfix};`
		),
		"};\n"
	]),
	"define_enums": [
		"enum class EntryType {",
		apply_format_data.map(entry => "\t" + entry.typename + ","),
		"};",
		"enum class Field {",
		mutable_fields.map(field => `\t${field.typename}_${field.name},`),
		"};"],
	"get_entry_type": ["EntryType get_Entry_EntryType(EntryID id) {",
		"\tString type = get_Entry_type(id);",
		apply_format_data
			.filter(entry => entry.type.length != 0)
			.map(entry => [
				`\tif (${entry.type.map(type => `type == "${type}"`).join(" || ")}) return EntryType::${entry.typename};`
			]),
		"return EntryType::Unknown;",
		"}"],
	"load_structs": [
		"Entry* load_type(String type, FileAccess* file, uint64_t next_chunk) {",
		apply_format_data
			.filter(entry => entry.type.length != 0)
			.map(entry => [
				entry.type[0] != "_" ? `\tif (${entry.type.map(type => `type == "${type}"`).join(" || ")}) {` : "",
				`\t\t${entry.typename}* entry = new ${entry.typename}();`,
				entry.fields.map(field =>
					`\t\t${field.load_func(`entry->${field.name}`, "file", "next_chunk")};`
				),
				"\t\treturn entry;",
				entry.type[0] != "_" ? "\t}" : ""
			]),
		"}"
	],
	"emit_all_modified": [
		"private:",
		apply_format_data
			.map(entry => [
				`void emit_${entry.typename}_modifed(Event event) {`,
				"event.event_type = WRL::Event::Type::Modified;",
				entry.fields.filter(field => !field.static).map(field => [
					`event.field = Field::${entry.typename}_${field.name};`,
					`_wrl_event(event);`
				]),
				"}"
			]),
		"protected:",
		`void _wrl_emit_modified(WRL::Event event) {`,
		"emit_Entry_modifed(event);",
		apply_format_data
			.filter(entry => entry.type.length != 0)
			.map(entry => [
				`if (event.entry_type == WRL::EntryType::${entry.typename}) return emit_${entry.typename}_modifed(event);`
			]),
		"}"
	],
	"declare_widgets": widget_fields.map(field => `${field.widget}* ${field.typename}_${field.name}_widget;`),
	"init_widgets": [
		apply_format_data
			.map(entry => [
				`void init_${entry.typename}_widgets() {`,
				entry.fields.filter(field => field.widget != undefined).map(field => [
					`\t{Label* label = memnew(Label);vbox->add_child(label);label->set_text("${field.name}");}`,
					`\t${entry.typename}_${field.name}_widget = memnew(${field.widget});`,
					field.static ?
						[
							`\t${entry.typename}_${field.name}_widget->set_enabled(false);`,
							`\t${entry.typename}_${field.name}_widget->input_value(wrl->get_${entry.typename}_${field.name}(selected));`
						] :
						`\t${entry.typename}_${field.name}_widget->output_value=[this](${field.extern_type} value, bool commit) { wrl->set_${entry.typename}_${field.name}(selected, value, commit); };`,
					`\tvbox->add_child(${entry.typename}_${field.name}_widget);\n`
				]),
				"}"
			]),
		`void init_widgets(WRL::EntryType type) {`,
		"init_Entry_widgets();",
		apply_format_data
			.filter(entry => entry.type.length != 0)
			.map(entry => [
				`if (type == WRL::EntryType::${entry.typename}) return init_${entry.typename}_widgets();`
			]),
		"}"
	],
	"update_widgets": [
		"void update_widgets(const WRL::Event& event) {",
		widget_fields.filter(field => !field.static).map(field => `\tif (event.field == WRL::Field::${field.typename}_${field.name}) ${field.typename}_${field.name}_widget->input_value(wrl->get_${field.typename}_${field.name}(event.id)); `),
		"}"
	]
};

Object.entries(generated_includes).map(([path, data]) => Deno.writeTextFile(`./${path}.gen.ipp`, data.flat(5).join("\n")));