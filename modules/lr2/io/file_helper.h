#pragma once

#include "core/io/file_access.h"

inline Vector2 get_vector2(Ref<FileAccess> f) {
	float x = f->get_float();
	float y = f->get_float();
	return Vector2(x, y);
}
inline void store_vector2(Ref<FileAccess> f, Vector2 v) {
	f->store_float(v.x);
	f->store_float(v.y);
}
inline Vector3 get_vector3(Ref<FileAccess> f) {
	float x = f->get_float();
	float y = f->get_float();
	float z = f->get_float();
	return Vector3(x, y, z);
}
inline void store_vector3(Ref<FileAccess> f, Vector3 v) {
	f->store_float(v.x);
	f->store_float(v.y);
	f->store_float(v.z);
}
inline Quaternion get_quaternion(Ref<FileAccess> f) {
	float x = f->get_float();
	float y = f->get_float();
	float z = f->get_float();
	float w = f->get_float();
	return Quaternion(x, y, z, w);
}
inline void store_quaternion(Ref<FileAccess> f, Quaternion q) {
	f->store_float(q.x);
	f->store_float(q.y);
	f->store_float(q.z);
	f->store_float(q.w);
}
inline Color get_colour(Ref<FileAccess> f) {
	float r = f->get_float();
	float g = f->get_float();
	float b = f->get_float();
	float a = f->get_float();
	return Color(r, g, b, a);
}

inline String get_string(Ref<FileAccess> f, int length) {
	CharString cs;
	cs.resize(length + 1);
	f->get_buffer((uint8_t*)cs.ptr(), length);
	cs[length] = 0;

	String ret;
	ret.parse_utf8(cs.ptr());

	return ret;
}
inline void store_string(Ref<FileAccess> f, String str, int length) {
	int end = f->get_position() + length;
	CharString cs = str.ascii();
	f->store_buffer((uint8_t*)cs.ptr(), cs.length());
	while (f->get_position() < end) {
		f->store_8(0);
	}
}