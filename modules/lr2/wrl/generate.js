const data_types = {
	"uint8*": field => ({
		storage_type: "Vector<uint8_t>", variant: "PACKED_BYTE_ARRAY", load_func: (field_address, file, next_chunk) => [
			`${field_address}.resize(${next_chunk} - ${file}->get_position());`,
			`\t\tfile->get_buffer(${field_address}.ptrw(), ${field_address}.size())`
		].join("\n")
	}),
	"uint8[]": field => ({
		storage_type: "uint8_t", extern_type: "Vector<uint8_t>", postfix: `[${field.length}]`, variant: "PACKED_BYTE_ARRAY",
		load_func: (field_address, file) => `${file}->get_buffer(${field_address}, ${field.length})`,
		getter: field_address => [
			"\tVector<uint8_t> v;",
			`\tv.resize(${field.length});`,
			`\tmemcpy(v.ptrw(), ${field_address}, ${field.length});`,
			`\treturn v;`
		],
		setter: (field_address, value) => [`\tmemcpy(${field_address}, ${value}.ptrw(), ${field.length});`]
	}),
	"uint32": { storage_type: "uint32_t", variant: "INT", load_func: (field_address, file) => `${field_address} = ${file}->get_32()` },
	"float": { variant: "FLOAT", load_func: (field_address, file) => `${field_address} = ${file}->get_float()`, },
	"String": field => ({ variant: "STRING", load_func: (field_address, file) => `${field_address} = get_string(${file}, ${field.length})` }),
	"Vector2": { variant: "VECTOR2", load_func: (field_address, file) => `${field_address} = get_vector2(${file})` },
	"Vector3": { variant: "VECTOR3", load_func: (field_address, file) => `${field_address} = get_vector3(${file})` },
	"Quaternion": { variant: "QUATERNION", load_func: (field_address, file) => `${field_address} = get_quaternion(${file})` }
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

const convert_from_record = Object.entries(entry_types).map(([typename, entry], i, array) => ({ ...entry, typename: typename, type: convertToArray(entry.type), parent: i == 0 ? "Object" : array[0][0] }));

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

const generated_includes = {
	"define_structs": apply_format_data.map((entry) => [
		`struct ${entry.typename} : public ${entry.parent} {`,
		`\tGDCLASS(${entry.typename}, ${entry.parent})`,
		"\n  public:",
		entry.fields.map(field =>
			`\t${field.storage_type} ${field.name}${field.postfix};`
		), "",
		entry.fields.map(field => [
			`\t${field.extern_type} get_${field.name} () {`,
			field.getter(field.name),
			"\t}",
			`\tvoid set_${field.name}(${field.extern_type} value){`,
			field.setter(field.name, "value"),
			"\t}\n"
		]),
		"\n  protected:",
		"\tstatic void _bind_methods() {",
		entry.fields.map(field => [
			`\t\tClassDB::bind_method(D_METHOD("get_${field.name}"), &${entry.typename}::get_${field.name});`,
			`\t\tClassDB::bind_method(D_METHOD("set_${field.name}", "${field.name}"), &${entry.typename}::set_${field.name});`,
			`\t\tADD_PROPERTY(PropertyInfo(Variant::${field.variant}, "${field.name}"), "set_${field.name}", "get_${field.name}");\n`,
		]),
		"\t}",
		"};\n"
	]),
	"load_structs": [
		"Entry* load_type(String type, FileAccess* file, uint64_t next_chunk) {",
		apply_format_data
			.filter(entry => entry.type.length != 0)
			.map(entry => [
				entry.type[0] != "_" ? `\tif (${entry.type.map(type => `type == "${type}"`).join(" || ")}) {` : "",
				`\t\t${entry.typename}* entry = memnew(${entry.typename});`,
				entry.fields.map(field =>
					`\t\t${field.load_func(`entry->${field.name}`, "file", "next_chunk")};`
				),
				"\t\treturn entry;",
				entry.type[0] != "_" ? "\t}" : ""
			]),
		"}"
	],
};

Object.entries(generated_includes).map(([path, data]) => Deno.writeTextFile(`./${path}.gen.ipp`, data.flat(5).join("\n")));