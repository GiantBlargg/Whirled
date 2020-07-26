using Godot;

namespace LR2.WRL {

	public class cLegoTerrain : WRLEntry {
		public override string Type => "cLegoTerrain";
		public override uint U => 3;
		public override uint Length => 0x164;

		public override Node Node => model;

		TDF model = new TDF();
		float u1, u2;
		uint u3, u4;
		byte[] u5, u6;

		public override void Load(File file) {
			model.Transform = file.GetTransform();
			u1 = file.GetFloat();
			u2 = file.GetFloat();
			u3 = file.Get32();
			model.modelPath = file.GetFixedString(0x80);
			u4 = file.Get32();
			model.Scale = file.GetVector3();
			u5 = file.GetBuffer(0x10);
			model.textureScale = file.GetVector2();
			u6 = file.GetBuffer(0x60);
		}

		public override void Save(File file) {
			file.StoreTransform(model.Transform);
			file.StoreFloat(u1);
			file.StoreFloat(u2);
			file.Store32(u3);
			file.StoreFixedString(model.modelPath, 0x80);
			file.Store32(u4);
			file.StoreVector3(model.Scale);
			file.StoreBuffer(u5);
			file.StoreVector2(model.textureScale);
			file.StoreBuffer(u6);
		}
	}
}

public class Terrain : CommonWRL {
	public override string type { get { return "cLegoTerrain"; } }
	public override uint u { get { return 3; } }
	public override uint byteLength { get { return 0x164; } }

	TDF model;

	uint u1;
	byte[] u2, u3;

	public override void Load(File file, uint u, uint length) {
		if (u != this.u) {
			GD.PrintErr("u mismatch! expected :", this.u, " got ", u);
		}
		if (length != byteLength) {
			GD.PrintErr("Length mismatch! expected :", byteLength, " got ", length);
		}

		LoadCommon(file);

		model = new TDF();

		model.Transform = file.GetTransform();

		if (0.5f != file.GetFloat() || 0.5f != file.GetFloat() || 0 != file.Get32()) {
			GD.PrintErr("Wrong Value!");
		}

		model.modelPath = file.GetFixedString(0x80);

		u1 = file.Get32();

		model.Scale = file.GetVector3();

		u2 = file.GetBuffer(0x10);

		model.textureScale = file.GetVector2();

		u3 = file.GetBuffer(0x60);

		AddChild(model);
	}

	public override void Save(File file) {
		SaveCommon(file);

		file.StoreTransform(model.Transform);

		file.StoreFloat(0.5f);
		file.StoreFloat(0.5f);
		file.Store32(0);

		file.StoreFixedString(model.modelPath, 0x80);

		file.Store32(u1);

		file.StoreVector3(model.Scale);

		file.StoreBuffer(u2);

		file.StoreVector2(model.textureScale);

		file.StoreBuffer(u3);
	}
}
