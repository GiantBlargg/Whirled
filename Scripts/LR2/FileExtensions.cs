using Godot;
using System;

public static class FileExtensions {
	public static string GetFixedString(this File file, int length) {
		var buffer = file.GetBuffer(length);
		return System.Text.Encoding.ASCII.GetString(buffer, 0, length).Split(new char[] { '\0' }, 2)[0];
	}
	public static void StoreFixedString(this File file, string str, int length) {
		var buffer = System.Text.Encoding.ASCII.GetBytes(str);
		Array.Resize(ref buffer, length);
		file.StoreBuffer(buffer);
	}

	public static Vector2 GetVector2(this File file) {
		return new Vector2(file.GetFloat(), file.GetFloat());
	}
	public static void StoreVector2(this File file, Vector2 vector) {
		file.StoreFloat(vector.x);
		file.StoreFloat(vector.y);
	}

	public static Vector3 GetVector3(this File file) {
		float x = file.GetFloat();
		float y = file.GetFloat();
		float z = file.GetFloat();
		return new Vector3(x, y, z);
	}
	public static void StoreVector3(this File file, Vector3 vector) {
		file.StoreFloat(vector.x);
		file.StoreFloat(vector.y);
		file.StoreFloat(vector.z);
	}

	public static Color GetColorRGBAf(this File file) {
		return new Color(file.GetFloat(), file.GetFloat(), file.GetFloat(), file.GetFloat());
	}

	public static Quat GetQuat(this File file) {
		return new Quat(file.GetFloat(), file.GetFloat(), file.GetFloat(), file.GetFloat());
	}
	public static void StoreQuat(this File file, Quat quat) {
		quat = quat.Normalized();
		file.StoreFloat(quat.x);
		file.StoreFloat(quat.y);
		file.StoreFloat(quat.z);
		file.StoreFloat(quat.w);
	}

	public static Transform GetTransform(this File file) {
		var origin = file.GetVector3();
		var rot = file.GetQuat();
		return new Transform(rot, origin);
	}
	public static void StoreTransform(this File file, Transform transform) {
		file.StoreVector3(transform.origin);
		file.StoreQuat(new Quat(transform.basis));
	}
}