struct Entry : public Object {
	GDCLASS(Entry, Object)

  public:
	String type;
	uint32_t u;
	uint32_t layer;
	String name;
	String binding;

	String get_type() { return type; }
	void set_type(String value) { type = value; }

	uint32_t get_u() { return u; }
	void set_u(uint32_t value) { u = value; }

	uint32_t get_layer() { return layer; }
	void set_layer(uint32_t value) { layer = value; }

	String get_name() { return name; }
	void set_name(String value) { name = value; }

	String get_binding() { return binding; }
	void set_binding(String value) { binding = value; }

  protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("get_type"), &Entry::get_type);
		ClassDB::bind_method(D_METHOD("set_type", "type"), &Entry::set_type);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "type"), "set_type", "get_type");

		ClassDB::bind_method(D_METHOD("get_u"), &Entry::get_u);
		ClassDB::bind_method(D_METHOD("set_u", "u"), &Entry::set_u);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "u"), "set_u", "get_u");

		ClassDB::bind_method(D_METHOD("get_layer"), &Entry::get_layer);
		ClassDB::bind_method(D_METHOD("set_layer", "layer"), &Entry::set_layer);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "layer"), "set_layer", "get_layer");

		ClassDB::bind_method(D_METHOD("get_name"), &Entry::get_name);
		ClassDB::bind_method(D_METHOD("set_name", "name"), &Entry::set_name);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "set_name", "get_name");

		ClassDB::bind_method(D_METHOD("get_binding"), &Entry::get_binding);
		ClassDB::bind_method(D_METHOD("set_binding", "binding"), &Entry::set_binding);
		ADD_PROPERTY(
			PropertyInfo(Variant::STRING, "binding", PROPERTY_HINT_NODE_PATH_VALID_TYPES), "set_binding",
			"get_binding");
	}
};

struct GeneralStatic : public Entry {
	GDCLASS(GeneralStatic, Entry)

  public:
	Vector3 position;
	Quaternion rotation;
	uint32_t u1;
	uint32_t u2;
	uint32_t collision_sound;
	String model;

	Vector3 get_position() { return position; }
	void set_position(Vector3 value) { position = value; }

	Quaternion get_rotation() { return rotation; }
	void set_rotation(Quaternion value) { rotation = value; }

	uint32_t get_u1() { return u1; }
	void set_u1(uint32_t value) { u1 = value; }

	uint32_t get_u2() { return u2; }
	void set_u2(uint32_t value) { u2 = value; }

	uint32_t get_collision_sound() { return collision_sound; }
	void set_collision_sound(uint32_t value) { collision_sound = value; }

	String get_model() { return model; }
	void set_model(String value) { model = value; }

  protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("get_position"), &GeneralStatic::get_position);
		ClassDB::bind_method(D_METHOD("set_position", "position"), &GeneralStatic::set_position);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "position"), "set_position", "get_position");

		ClassDB::bind_method(D_METHOD("get_rotation"), &GeneralStatic::get_rotation);
		ClassDB::bind_method(D_METHOD("set_rotation", "rotation"), &GeneralStatic::set_rotation);
		ADD_PROPERTY(PropertyInfo(Variant::QUATERNION, "rotation"), "set_rotation", "get_rotation");

		ClassDB::bind_method(D_METHOD("get_u1"), &GeneralStatic::get_u1);
		ClassDB::bind_method(D_METHOD("set_u1", "u1"), &GeneralStatic::set_u1);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "u1"), "set_u1", "get_u1");

		ClassDB::bind_method(D_METHOD("get_u2"), &GeneralStatic::get_u2);
		ClassDB::bind_method(D_METHOD("set_u2", "u2"), &GeneralStatic::set_u2);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "u2"), "set_u2", "get_u2");

		ClassDB::bind_method(D_METHOD("get_collision_sound"), &GeneralStatic::get_collision_sound);
		ClassDB::bind_method(D_METHOD("set_collision_sound", "collision_sound"), &GeneralStatic::set_collision_sound);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_sound"), "set_collision_sound", "get_collision_sound");

		ClassDB::bind_method(D_METHOD("get_model"), &GeneralStatic::get_model);
		ClassDB::bind_method(D_METHOD("set_model", "model"), &GeneralStatic::set_model);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "model", PROPERTY_HINT_FILE, "*.md2"), "set_model", "get_model");
	}
};

struct GeneralMobile : public Entry {
	GDCLASS(GeneralMobile, Entry)

  public:
	Vector3 position;
	Quaternion rotation;
	uint32_t u1;
	uint32_t u2;
	uint32_t collision_sound;
	float weight;
	Vector3 u3;
	String model;
	uint32_t u4;

	Vector3 get_position() { return position; }
	void set_position(Vector3 value) { position = value; }

	Quaternion get_rotation() { return rotation; }
	void set_rotation(Quaternion value) { rotation = value; }

	uint32_t get_u1() { return u1; }
	void set_u1(uint32_t value) { u1 = value; }

	uint32_t get_u2() { return u2; }
	void set_u2(uint32_t value) { u2 = value; }

	uint32_t get_collision_sound() { return collision_sound; }
	void set_collision_sound(uint32_t value) { collision_sound = value; }

	float get_weight() { return weight; }
	void set_weight(float value) { weight = value; }

	Vector3 get_u3() { return u3; }
	void set_u3(Vector3 value) { u3 = value; }

	String get_model() { return model; }
	void set_model(String value) { model = value; }

	uint32_t get_u4() { return u4; }
	void set_u4(uint32_t value) { u4 = value; }

  protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("get_position"), &GeneralMobile::get_position);
		ClassDB::bind_method(D_METHOD("set_position", "position"), &GeneralMobile::set_position);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "position"), "set_position", "get_position");

		ClassDB::bind_method(D_METHOD("get_rotation"), &GeneralMobile::get_rotation);
		ClassDB::bind_method(D_METHOD("set_rotation", "rotation"), &GeneralMobile::set_rotation);
		ADD_PROPERTY(PropertyInfo(Variant::QUATERNION, "rotation"), "set_rotation", "get_rotation");

		ClassDB::bind_method(D_METHOD("get_u1"), &GeneralMobile::get_u1);
		ClassDB::bind_method(D_METHOD("set_u1", "u1"), &GeneralMobile::set_u1);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "u1"), "set_u1", "get_u1");

		ClassDB::bind_method(D_METHOD("get_u2"), &GeneralMobile::get_u2);
		ClassDB::bind_method(D_METHOD("set_u2", "u2"), &GeneralMobile::set_u2);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "u2"), "set_u2", "get_u2");

		ClassDB::bind_method(D_METHOD("get_collision_sound"), &GeneralMobile::get_collision_sound);
		ClassDB::bind_method(D_METHOD("set_collision_sound", "collision_sound"), &GeneralMobile::set_collision_sound);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_sound"), "set_collision_sound", "get_collision_sound");

		ClassDB::bind_method(D_METHOD("get_weight"), &GeneralMobile::get_weight);
		ClassDB::bind_method(D_METHOD("set_weight", "weight"), &GeneralMobile::set_weight);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "weight"), "set_weight", "get_weight");

		ClassDB::bind_method(D_METHOD("get_u3"), &GeneralMobile::get_u3);
		ClassDB::bind_method(D_METHOD("set_u3", "u3"), &GeneralMobile::set_u3);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "u3"), "set_u3", "get_u3");

		ClassDB::bind_method(D_METHOD("get_model"), &GeneralMobile::get_model);
		ClassDB::bind_method(D_METHOD("set_model", "model"), &GeneralMobile::set_model);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "model", PROPERTY_HINT_FILE, "*.md2"), "set_model", "get_model");

		ClassDB::bind_method(D_METHOD("get_u4"), &GeneralMobile::get_u4);
		ClassDB::bind_method(D_METHOD("set_u4", "u4"), &GeneralMobile::set_u4);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "u4"), "set_u4", "get_u4");
	}
};

struct LegoTerrain : public Entry {
	GDCLASS(LegoTerrain, Entry)

  public:
	Vector3 position;
	Quaternion rotation;
	float u1;
	float u2;
	uint32_t u3;
	String model;
	uint32_t u4;
	Vector3 scale;
	Vector<uint8_t> u5;
	Vector2 texture_scale;
	Vector<uint8_t> u6;

	Vector3 get_position() { return position; }
	void set_position(Vector3 value) { position = value; }

	Quaternion get_rotation() { return rotation; }
	void set_rotation(Quaternion value) { rotation = value; }

	float get_u1() { return u1; }
	void set_u1(float value) { u1 = value; }

	float get_u2() { return u2; }
	void set_u2(float value) { u2 = value; }

	uint32_t get_u3() { return u3; }
	void set_u3(uint32_t value) { u3 = value; }

	String get_model() { return model; }
	void set_model(String value) { model = value; }

	uint32_t get_u4() { return u4; }
	void set_u4(uint32_t value) { u4 = value; }

	Vector3 get_scale() { return scale; }
	void set_scale(Vector3 value) { scale = value; }

	Vector<uint8_t> get_u5() { return u5; }
	void set_u5(const Vector<uint8_t>& value) { u5 = value; }

	Vector2 get_texture_scale() { return texture_scale; }
	void set_texture_scale(Vector2 value) { texture_scale = value; }

	Vector<uint8_t> get_u6() { return u6; }
	void set_u6(const Vector<uint8_t>& value) { u6 = value; }

  protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("get_position"), &LegoTerrain::get_position);
		ClassDB::bind_method(D_METHOD("set_position", "position"), &LegoTerrain::set_position);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "position"), "set_position", "get_position");

		ClassDB::bind_method(D_METHOD("get_rotation"), &LegoTerrain::get_rotation);
		ClassDB::bind_method(D_METHOD("set_rotation", "rotation"), &LegoTerrain::set_rotation);
		ADD_PROPERTY(PropertyInfo(Variant::QUATERNION, "rotation"), "set_rotation", "get_rotation");

		ClassDB::bind_method(D_METHOD("get_u1"), &LegoTerrain::get_u1);
		ClassDB::bind_method(D_METHOD("set_u1", "u1"), &LegoTerrain::set_u1);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "u1"), "set_u1", "get_u1");

		ClassDB::bind_method(D_METHOD("get_u2"), &LegoTerrain::get_u2);
		ClassDB::bind_method(D_METHOD("set_u2", "u2"), &LegoTerrain::set_u2);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "u2"), "set_u2", "get_u2");

		ClassDB::bind_method(D_METHOD("get_u3"), &LegoTerrain::get_u3);
		ClassDB::bind_method(D_METHOD("set_u3", "u3"), &LegoTerrain::set_u3);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "u3"), "set_u3", "get_u3");

		ClassDB::bind_method(D_METHOD("get_model"), &LegoTerrain::get_model);
		ClassDB::bind_method(D_METHOD("set_model", "model"), &LegoTerrain::set_model);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "model", PROPERTY_HINT_DIR), "set_model", "get_model");

		ClassDB::bind_method(D_METHOD("get_u4"), &LegoTerrain::get_u4);
		ClassDB::bind_method(D_METHOD("set_u4", "u4"), &LegoTerrain::set_u4);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "u4"), "set_u4", "get_u4");

		ClassDB::bind_method(D_METHOD("get_scale"), &LegoTerrain::get_scale);
		ClassDB::bind_method(D_METHOD("set_scale", "scale"), &LegoTerrain::set_scale);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "scale"), "set_scale", "get_scale");

		ClassDB::bind_method(D_METHOD("get_u5"), &LegoTerrain::get_u5);
		ClassDB::bind_method(D_METHOD("set_u5", "u5"), &LegoTerrain::set_u5);
		ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "u5", PROPERTY_HINT_LENGTH, "16"), "set_u5", "get_u5");

		ClassDB::bind_method(D_METHOD("get_texture_scale"), &LegoTerrain::get_texture_scale);
		ClassDB::bind_method(D_METHOD("set_texture_scale", "texture_scale"), &LegoTerrain::set_texture_scale);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "texture_scale"), "set_texture_scale", "get_texture_scale");

		ClassDB::bind_method(D_METHOD("get_u6"), &LegoTerrain::get_u6);
		ClassDB::bind_method(D_METHOD("set_u6", "u6"), &LegoTerrain::set_u6);
		ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "u6", PROPERTY_HINT_LENGTH, "96"), "set_u6", "get_u6");
	}
};

struct SkyBox : public Entry {
	GDCLASS(SkyBox, Entry)

  public:
	String model;

	String get_model() { return model; }
	void set_model(String value) { model = value; }

  protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("get_model"), &SkyBox::get_model);
		ClassDB::bind_method(D_METHOD("set_model", "model"), &SkyBox::set_model);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "model", PROPERTY_HINT_FILE, "*.md2"), "set_model", "get_model");
	}
};

struct Unknown : public Entry {
	GDCLASS(Unknown, Entry)

  public:
	Vector<uint8_t> data;

	Vector<uint8_t> get_data() { return data; }
	void set_data(Vector<uint8_t> value) { data = value; }

  protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("get_data"), &Unknown::get_data);
		ClassDB::bind_method(D_METHOD("set_data", "data"), &Unknown::set_data);
		ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "data"), "set_data", "get_data");
	}
};
