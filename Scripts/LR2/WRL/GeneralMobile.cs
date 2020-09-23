using Godot;
using System.Collections.Generic;

namespace LR2.WRL {
	public class cGeneralMobile : WRLEntry {
		public override string Type => "cGeneralMobile";
		public override uint U => 0;
		public override uint Length => 0xf0;

		public override Node Node => model;
		public override CollisionObject3D selectCollider => model.collider;

		float u1, u2;
		uint SoundId;
		uint u3;

		float Mass;
		Vector3 Inertia;

		MDL2 model = new MDL2();

		public override void Load(File file) {
			model.Transform = file.GetTransform();

			u1 = file.GetFloat();
			u2 = file.GetFloat();

			SoundId = file.Get32();

			Mass = file.GetFloat();
			Inertia = file.GetVector3();

			model.modelPath = file.GetFixedString(0x80);

			u3 = file.Get32();
		}

		public override void Save(File file) {
			file.StoreTransform(model.Transform);

			file.StoreFloat(u1);
			file.StoreFloat(u2);
			file.Store32(SoundId);

			file.StoreFloat(Mass);

			file.StoreVector3(Inertia);

			file.StoreFixedString(model.modelPath, 0x80);

			file.Store32(u3);
		}

		public override List<ObjectProperty> GetProperties() {
			var p = base.GetProperties();
			p.Add(new ObjectProperty<Transform>(() => model.Transform));
			p.Add(new ObjectProperty<float>(() => u1));
			p.Add(new ObjectProperty<float>(() => u2));
			p.Add(new ObjectProperty<uint>(() => SoundId));
			p.Add(new ObjectProperty<float>(() => Mass));
			p.Add(new ObjectProperty<Vector3>(() => Inertia));
			p.Add(new ObjectProperty<string>(() => model.modelPath));
			p.Add(new ObjectProperty<uint>(() => u3));
			return p;
		}
	}
}