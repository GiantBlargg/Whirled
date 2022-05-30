#include "wrl.hpp"

Vector<WRL::Format::Property> WRL::common_properties;

const HashMap<String, WRL::Format>& WRL::get_formats() {
	static HashMap<String, WRL::Format> formats;
	if (formats.is_empty()) {
		common_properties.push_back({Variant::INT, "layer"});
		common_properties.push_back({Variant::STRING, "name", 24});

		{
			Format format;
			format.properties.append_array(common_properties);

			format.properties.push_back({Variant::STRING, "binding", 24, Format::Property::Flags::EntryID});
			format.properties.push_back({Variant::VECTOR3, "position"});
			format.properties.push_back({Variant::QUATERNION, "rotation"});
			format.properties.push_back({Variant::INT, "u1"});
			format.properties.push_back({Variant::INT, "u2"});
			format.properties.push_back({Variant::INT, "collision_sound"});
			format.properties.push_back({Variant::STRING, "model", 0x80, Format::Property::Flags::File, ".md2"});

			format.type = "cGeneralStatic";
			format.u = 0;
			formats.insert(format.type, format);

			format.type = "cGoldenBrick";
			format.u = 1;
			formats.insert(format.type, format);
		}
		{
			Format format;
			format.properties.append_array(common_properties);

			format.properties.push_back({Variant::STRING, "binding", 24, Format::Property::Flags::EntryID});
			format.properties.push_back({Variant::VECTOR3, "position"});
			format.properties.push_back({Variant::QUATERNION, "rotation"});
			format.properties.push_back({Variant::INT, "u1"});
			format.properties.push_back({Variant::INT, "u2"});
			format.properties.push_back({Variant::INT, "collision_sound"});
			format.properties.push_back({Variant::FLOAT, "weight"});
			format.properties.push_back({Variant::VECTOR3, "u3"});
			format.properties.push_back({Variant::STRING, "model", 0x80, Format::Property::Flags::File, ".md2"});
			format.properties.push_back({Variant::INT, "u4"});

			format.type = "cGeneralMobile";
			format.u = 0;
			formats.insert(format.type, format);

			format.type = "cBonusPickup";
			format.u = 1;
			formats.insert(format.type, format);
		}
		{
			Format format;
			format.properties.append_array(common_properties);

			format.properties.push_back({Variant::STRING, "binding", 24, Format::Property::Flags::EntryID});
			format.properties.push_back({Variant::VECTOR3, "position"});
			format.properties.push_back({Variant::QUATERNION, "rotation"});
			format.properties.push_back({Variant::FLOAT, "u1"});
			format.properties.push_back({Variant::FLOAT, "u2"});
			format.properties.push_back({Variant::INT, "u3"});
			format.properties.push_back(
				{Variant::STRING, "model", 0x80, Format::Property::Flags::File, "/terrdata.tdf"});
			format.properties.push_back({Variant::INT, "u4"});
			format.properties.push_back({Variant::VECTOR3, "scale"});
			format.properties.push_back({Variant::PACKED_BYTE_ARRAY, "u5", 16});
			format.properties.push_back({Variant::VECTOR2, "texture_scale"});
			format.properties.push_back({Variant::PACKED_BYTE_ARRAY, "u6", 96});

			format.type = "cLegoTerrain";
			format.u = 3;
			formats.insert(format.type, format);
		}
		{
			Format format;
			format.properties.append_array(common_properties);

			format.properties.push_back({Variant::STRING, "binding", 24, Format::Property::Flags::EntryID});
			format.properties.push_back({Variant::STRING, "model", 0x80, Format::Property::Flags::File, ".md2"});

			format.type = "cSkyBox";
			format.u = 0;
			formats.insert(format.type, format);
		}
	}
	return formats;
}