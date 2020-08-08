using Godot;
using System.Collections.Generic;

namespace LR2.WRL {
	public class cRaceStartPos : WRLEntry {
		public override string Type => "cRaceStartPos";
		public override uint U => 1;
		public override uint Length => 0x54;

		Transform Transform;

		uint u1;

		public override void Load(File file) {
			Transform = file.GetTransform();
			u1 = file.Get32();
		}

		public override void Save(File file) {
			file.StoreTransform(Transform);
			file.Store32(u1);
		}

		public override List<ObjectProperty> GetProperties() {
			var p = base.GetProperties();
			p.Add(new ObjectProperty<Transform>(() => Transform));
			p.Add(new ObjectProperty<uint>(() => u1));
			return p;
		}
	}

	public class cFoyerStartPos : cRaceStartPos {
		public override string Type => "cFoyerStartPos";
	}
}