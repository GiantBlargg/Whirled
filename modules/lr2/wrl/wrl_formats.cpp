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
						{Variant::FLOAT, "_1"},
						{Variant::FLOAT, "_2"},
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
						{Variant::FLOAT, "_1"},
						{Variant::FLOAT, "_2"},
						{Variant::INT, "collision_sound"},
						{Variant::STRING, "model", 0x80, Format::Property::Flags::File, ".md2"},
					},
				.model = {"model", "position", "rotation"},
			},
			{
				.type = "cBonusVortex",
				.u = 1,
				.properties =
					{
						{Variant::STRING, "binding", 24, Format::Property::Flags::EntryID},
						{Variant::VECTOR3, "position"},
						{Variant::QUATERNION, "rotation"},
						{Variant::FLOAT, "_1"},
						{Variant::FLOAT, "_2"},
						{Variant::INT, "collision_sound"},
						{Variant::STRING, "model", 0x80, Format::Property::Flags::File, ".md2"},
						{Variant::INT, "difficulty"},
					},
				.model = {"model", "position", "rotation"},
			},
			{
				.type = "cThePits",
				.u = 1,
				.properties =
					{
						{Variant::STRING, "binding", 24, Format::Property::Flags::EntryID},
						{Variant::VECTOR3, "position"},
						{Variant::QUATERNION, "rotation"},
						{Variant::FLOAT, "_1"},
						{Variant::FLOAT, "_2"},
						{Variant::INT, "collision_sound"},
						{Variant::STRING, "model", 0x80, Format::Property::Flags::File, ".md2"},
						{Variant::FLOAT, "_3"},
					},
				.model = {"model", "position", "rotation"},
			},
			{
				.type = "cWeaponPickup",
				.u = 1,
				.properties =
					{
						{Variant::STRING, "binding", 24, Format::Property::Flags::EntryID},
						{Variant::VECTOR3, "position"},
						{Variant::QUATERNION, "rotation"},
						{Variant::FLOAT, "_1"},
						{Variant::FLOAT, "_2"},
						{Variant::INT, "collision_sound"},
						{Variant::STRING, "model", 0x80, Format::Property::Flags::File, ".md2"},
						{Variant::FLOAT, "_3"},
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
						{Variant::FLOAT, "_1"},
						{Variant::FLOAT, "_2"},
						{Variant::INT, "collision_sound"},
						{Variant::FLOAT, "weight"},
						{Variant::VECTOR3, "_3"},
						{Variant::STRING, "model", 0x80, Format::Property::Flags::File, ".md2"},
						{Variant::INT, "_4"},
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
						{Variant::FLOAT, "_1"},
						{Variant::FLOAT, "_2"},
						{Variant::INT, "collision_sound"},
						{Variant::FLOAT, "weight"},
						{Variant::VECTOR3, "_3"},
						{Variant::STRING, "model", 0x80, Format::Property::Flags::File, ".md2"},
						{Variant::INT, "_4"},
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
						{Variant::FLOAT, "_1"},
						{Variant::FLOAT, "_2"},
						{Variant::INT, "_3"},
						{Variant::STRING, "model", 0x80, Format::Property::Flags::File, "/terrdata.tdf"},
						{Variant::INT, "_4"},
						{Variant::VECTOR3, "scale"},
						{Variant::INT, "_5"},
						{Variant::INT, "_6"},
						{Variant::INT, "_7"},
						{Variant::INT, "_8"},
						{Variant::VECTOR2, "texture_scale"},
						{Variant::INT, "_9"},
						{Variant::INT, "_10"},
						{Variant::INT, "_11"},
						{Variant::INT, "_12"},
						{Variant::INT, "_13"},
						{Variant::INT, "_14"},
						{Variant::INT, "_15"},
						{Variant::INT, "_16"},
						{Variant::INT, "_17"},
						{Variant::INT, "_18"},
						{Variant::INT, "_19"},
						{Variant::INT, "_20"},
						{Variant::INT, "_21"},
						{Variant::INT, "_22"},
						{Variant::INT, "_23"},
						{Variant::INT, "_24"},
						{Variant::INT, "_25"},
						{Variant::INT, "_26"},
						{Variant::INT, "_27"},
						{Variant::INT, "_28"},
						{Variant::INT, "_29"},
						{Variant::INT, "_30"},
						{Variant::INT, "_31"},
						{Variant::INT, "_32"},
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