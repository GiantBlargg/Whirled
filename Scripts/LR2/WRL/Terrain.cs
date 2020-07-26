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
