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
			model.ModelPath = file.GetFixedString(0x80);
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
			file.StoreFixedString(model.ModelPath, 0x80);
			file.Store32(u4);
			file.StoreVector3(model.Scale);
			file.StoreBuffer(u5);
			file.StoreVector2(model.textureScale);
			file.StoreBuffer(u6);
		}

		public override System.Collections.Generic.List<ObjectProperty> GetProperties() {
			var p = base.GetProperties();
			p.Add(new ObjectProperty<Transform3D>(() => model.Transform));
			p.Add(new ObjectProperty<float>(() => u1));
			p.Add(new ObjectProperty<float>(() => u2));
			p.Add(new ObjectProperty<uint>(() => u3));
			p.Add(new ObjectProperty<string>(() => model.ModelPath));
			p.Add(new ObjectProperty<uint>(() => u4));
			p.Add(new ObjectProperty<Vector3>(() => model.Scale));
			p.Add(new ObjectProperty<byte[]>(() => u5));
			p.Add(new ObjectProperty<Vector2>(() => model.textureScale));
			p.Add(new ObjectProperty<byte[]>(() => u6));
			return p;
		}
	}
}
