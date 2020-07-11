using Godot;

public class ModelEntry : CommonWRL {
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

	public override Spatial GizmoTarget { get { return model; } }

	MDL2 model;

	public ModelEntry(string type) {
		if (type != "cGeneralStatic" && type != "cGoldenBrick")
			GD.PrintErr("ModelEntry wrong type: ", type);
		this.type = type;
	}

	public override void Load(File file, uint u, uint length) {
		if (u != this.u) {
			GD.PrintErr("u mismatch! expected :", this.u, " got ", u);
		}
		if (length != byteLength) {
			GD.PrintErr("Length mismatch! expected :", byteLength, " got ", length);
		}

		LoadCommon(file);

		model = new MDL2();

		model.Transform = file.GetTransform();

		u1 = file.GetFloat();
		u2 = file.GetFloat();

		soundId = file.Get32();

		model.modelPath = file.GetFixedString(0x80);

		AddChild(model);
	}

	public override void Save(File file) {
		SaveCommon(file);

		file.StoreTransform(model.Transform);

		file.StoreFloat(u1);
		file.StoreFloat(u2);

		file.Store32(soundId);

		file.StoreFixedString(model.modelPath, 0x80);
	}
}