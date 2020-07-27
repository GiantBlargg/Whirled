using Godot;

namespace LR2.WRL {

	public class cGeneralStatic : WRLEntry {
		public override string Type => "cGeneralStatic";
		public override uint U => 0;
		public override uint Length => 0xdc;

		public override Node Node => model;
		public override CollisionObject selectCollider => model.collider;

		[WRLProperty(flags = PropertyFlags.Safe)]
		public Transform transform { get { return model.Transform; } set { model.Transform = value; } }

		uint soundId;
		float u1, u2;

		MDL2 model = new MDL2();

		public override void Load(File file) {
			var transform = file.GetTransform();
			model.Transform = transform;

			u1 = file.GetFloat();
			u2 = file.GetFloat();
			soundId = file.Get32();

			model.modelPath = file.GetFixedString(0x80);
		}

		public override void Save(File file) {
			var transform = model.Transform;
			file.StoreTransform(transform);

			file.StoreFloat(u1);
			file.StoreFloat(u2);

			file.Store32(soundId);

			file.StoreFixedString(model.modelPath, 0x80);
		}
	}

	public class cGoldenBrick : cGeneralStatic {
		public override string Type => "cGoldenBrick";
		public override uint U => 1;
	}
}
