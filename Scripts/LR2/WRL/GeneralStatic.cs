using Godot;
using System.Collections.Generic;

namespace LR2.WRL {

	public class cGeneralStatic : WRLEntry {
		public override string Type => "cGeneralStatic";
		public override uint U => 0;
		public override uint Length => 0xdc;

		public override Node Node => model;
		public override CollisionObject3D selectCollider => model.collider;

		float u1, u2;
		uint SoundId;

		MDL2 model = new MDL2();

		public override void Load(File file) {
			model.Transform = file.GetTransform();

			u1 = file.GetFloat();
			u2 = file.GetFloat();
			SoundId = file.Get32();

			model.modelPath = file.GetFixedString(0x80);
		}

		public override void Save(File file) {
			file.StoreTransform(model.Transform);

			file.StoreFloat(u1);
			file.StoreFloat(u2);
			file.Store32(SoundId);

			file.StoreFixedString(model.modelPath, 0x80);
		}

		public override List<ObjectProperty> GetProperties() {
			var p = base.GetProperties();
			p.Add(new ObjectProperty<Transform3D>(() => model.Transform));
			p.Add(new ObjectProperty<float>(() => u1));
			p.Add(new ObjectProperty<float>(() => u2));
			p.Add(new ObjectProperty<uint>(() => SoundId));
			p.Add(new ObjectProperty<string>(() => model.modelPath));
			return p;
		}
	}

	public class cGoldenBrick : cGeneralStatic {
		public override string Type => "cGoldenBrick";
		public override uint U => 1;
	}
}
