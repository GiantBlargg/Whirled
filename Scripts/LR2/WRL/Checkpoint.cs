using Godot;
using System.Collections.Generic;

namespace LR2.WRL {
	public class cCheckPoint : WRLEntry {
		public override string Type => "cCheckPoint";
		public override uint U => 0;
		public override uint Length => 0x19c;

		public override Node Node => model;
		public override CollisionObject selectCollider => model.collider;

		float u1, u2;
		uint SoundId;

		uint id;
		uint[] Next = new uint[4];

		byte[] u3;

		Vector3[] Quad = new Vector3[4];

		byte[] u4;

		Vector3 Arrow;

		float u5;

		MDL2 model = new MDL2();

		public override void Load(File file) {
			model.Transform = file.GetTransform();

			u1 = file.GetFloat();
			u2 = file.GetFloat();
			SoundId = file.Get32();

			model.modelPath = file.GetFixedString(0x80);

			id = file.Get32();

			Next[0] = file.Get32();
			Next[1] = file.Get32();
			Next[2] = file.Get32();
			Next[3] = file.Get32();

			u3 = file.GetBuffer(0x18);

			Quad[0] = model.Transform.XformInv(file.GetVector3());
			Quad[1] = model.Transform.XformInv(file.GetVector3());
			Quad[2] = model.Transform.XformInv(file.GetVector3());
			Quad[3] = model.Transform.XformInv(file.GetVector3());

			u4 = file.GetBuffer(0x54);

			Arrow = model.Transform.XformInv(file.GetVector3());

			u5 = file.GetFloat();
		}
		public override void Save(File file) {
			file.StoreTransform(model.Transform);

			file.StoreFloat(u1);
			file.StoreFloat(u2);
			file.Store32(SoundId);

			file.StoreFixedString(model.modelPath, 0x80);

			file.Store32(id);
			file.Store32(Next[0]);
			file.Store32(Next[1]);
			file.Store32(Next[2]);
			file.Store32(Next[3]);

			file.StoreBuffer(u3);

			file.StoreVector3(model.Transform.Xform(Quad[0]));
			file.StoreVector3(model.Transform.Xform(Quad[1]));
			file.StoreVector3(model.Transform.Xform(Quad[2]));
			file.StoreVector3(model.Transform.Xform(Quad[3]));

			file.StoreBuffer(u4);

			file.StoreVector3(model.Transform.Xform(Arrow));

			file.StoreFloat(u5);
		}
		public override List<ObjectProperty> GetProperties() {
			var p = base.GetProperties();
			p.Add(new ObjectProperty<Transform>(() => model.Transform));
			p.Add(new ObjectProperty<float>(() => u1));
			p.Add(new ObjectProperty<float>(() => u2));
			p.Add(new ObjectProperty<uint>(() => SoundId));
			p.Add(new ObjectProperty<string>(() => model.modelPath));
			p.Add(new ObjectProperty<uint>(() => id));
			p.Add(new ObjectProperty<uint[]>(() => Next));
			p.Add(new ObjectProperty<byte[]>(() => u3));
			p.Add(new ObjectProperty<Vector3[]>(() => Quad));
			p.Add(new ObjectProperty<byte[]>(() => u4));
			p.Add(new ObjectProperty<Vector3>(() => Arrow));
			p.Add(new ObjectProperty<float>(() => u5));
			return p;
		}
	}
}