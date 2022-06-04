#include "wrl.hpp"

Vector<WRL::Format::Property> WRL::common_properties{{Variant::INT, "layer"}, {Variant::STRING, "name", 24}};

const HashMap<String, WRL::Format>& WRL::get_formats() {
	static HashMap<String, WRL::Format> formats;
	if (formats.is_empty()) {
		Vector<WRL::Format> partial_formats{
			{
				.type = "cGeneralStatic",
				.u = 0,
				.properties =
					{
						{Variant::STRING, "binding", 24, Format::Property::Flags::EntryID},
						{Variant::VECTOR3, "position"},
						{Variant::QUATERNION, "rotation"},
						{Variant::INT, "u1"},
						{Variant::INT, "u2"},
						{Variant::INT, "collision_sound"},
						{Variant::STRING, "model", 0x80, Format::Property::Flags::File, ".md2"},
					},
				.model = {"model", "position", "rotation"},
			},
			{
				.type = "cGoldenBrick",
				.u = 1,
				.properties =
					{
						{Variant::STRING, "binding", 24, Format::Property::Flags::EntryID},
						{Variant::VECTOR3, "position"},
						{Variant::QUATERNION, "rotation"},
						{Variant::INT, "u1"},
						{Variant::INT, "u2"},
						{Variant::INT, "collision_sound"},
						{Variant::STRING, "model", 0x80, Format::Property::Flags::File, ".md2"},
					},
				.model = {"model", "position", "rotation"},
			},
			{
				.type = "cGeneralMobile",
				.u = 0,
				.properties =
					{
						{Variant::STRING, "binding", 24, Format::Property::Flags::EntryID},
						{Variant::VECTOR3, "position"},
						{Variant::QUATERNION, "rotation"},
						{Variant::INT, "u1"},
						{Variant::INT, "u2"},
						{Variant::INT, "collision_sound"},
						{Variant::FLOAT, "weight"},
						{Variant::VECTOR3, "u3"},
						{Variant::STRING, "model", 0x80, Format::Property::Flags::File, ".md2"},
						{Variant::INT, "u4"},
					},
				.model = {"model", "position", "rotation"},
			},
			{
				.type = "cBonusPickup",
				.u = 1,
				.properties =
					{
						{Variant::STRING, "binding", 24, Format::Property::Flags::EntryID},
						{Variant::VECTOR3, "position"},
						{Variant::QUATERNION, "rotation"},
						{Variant::INT, "u1"},
						{Variant::INT, "u2"},
						{Variant::INT, "collision_sound"},
						{Variant::FLOAT, "weight"},
						{Variant::VECTOR3, "u3"},
						{Variant::STRING, "model", 0x80, Format::Property::Flags::File, ".md2"},
						{Variant::INT, "u4"},
					},
				.model = {"model", "position", "rotation"},
			},
			{
				.type = "cLegoTerrain",
				.u = 3,
				.properties =
					{
						{Variant::STRING, "binding", 24, Format::Property::Flags::EntryID},
						{Variant::VECTOR3, "position"},
						{Variant::QUATERNION, "rotation"},
						{Variant::FLOAT, "u1"},
						{Variant::FLOAT, "u2"},
						{Variant::INT, "u3"},
						{Variant::STRING, "model", 0x80, Format::Property::Flags::File, "/terrdata.tdf"},
						{Variant::INT, "u4"},
						{Variant::VECTOR3, "scale"},
						{Variant::PACKED_BYTE_ARRAY, "u5", 16},
						{Variant::VECTOR2, "texture_scale"},
						{Variant::PACKED_BYTE_ARRAY, "u6", 96},
					},
				.model = {"model", "position", "rotation", "scale", Format::Model::Type::Terrain, {"texture_scale"}},
			},
			{
				.type = "cSkyBox",
				.u = 0,
				.properties =
					{
						{Variant::STRING, "binding", 24, Format::Property::Flags::EntryID},
						{Variant::STRING, "model", 0x80, Format::Property::Flags::File, ".md2"},
					},
				.model = {.model = "model", .type = Format::Model::Type::Skybox},
			}};

		for (auto& format : partial_formats) {
			auto unique_properties = format.properties;
			format.properties = common_properties.duplicate();
			format.properties.append_array(unique_properties);
			formats.insert(format.type, format);
		}
	}
	return formats;
}