using Godot;

public class cGeneralStatic : WRLEntry {
	public override string Type => "cGeneralStatic";
	public override uint U => 0;
	public override uint Length => 0xdc;

	public override Node Node => model;

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

public class OldModelEntry : CommonWRL {
	public override string type { get; }
	public override uint u {
		get {
			switch (type) {
				default:
				case "cGeneralStatic":
					return 0;
				case "cGoldenBrick":
					return 1;
			}
		}
	}
	public override uint byteLength { get { return 0xdc; } }

	private uint soundId;

	float u1, u2;

	public override GizmoTarget GizmoTarget { get; }

	public override Control PropertyControl { get; }

	MDL2 model;

	public OldModelEntry(string type) {
		if (type != "cGeneralStatic" && type != "cGoldenBrick")
			GD.PrintErr("ModelEntry wrong type: ", type);
		this.type = type;

		GizmoTarget = new GizmoTarget();
		AddChild(GizmoTarget);

		PropertyControl = new VBoxContainer();
		PropertyControl.SizeFlagsHorizontal = (int)Control.SizeFlags.ExpandFill;
		var transformUI = new TransformUI(GizmoTarget);
		PropertyControl.AddChild(transformUI);
	}

	public override void Load(File file, uint u, uint length) {
		if (u != this.u) {
			GD.PrintErr("u mismatch! expected :", this.u, " got ", u);
		}
		if (length != byteLength) {
			GD.PrintErr("Length mismatch! expected :", byteLength, " got ", length);
		}

		LoadCommon(file);

		GizmoTarget.Transform = file.GetTransform();

		u1 = file.GetFloat();
		u2 = file.GetFloat();

		soundId = file.Get32();

		model = new MDL2();

		model.modelPath = file.GetFixedString(0x80);

		GizmoTarget.AddChild(model);
	}

	public override void Save(File file) {
		SaveCommon(file);

		file.StoreTransform(GizmoTarget.Transform);

		file.StoreFloat(u1);
		file.StoreFloat(u2);

		file.Store32(soundId);

		file.StoreFixedString(model.modelPath, 0x80);
	}
}