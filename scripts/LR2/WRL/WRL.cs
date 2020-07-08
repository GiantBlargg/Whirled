using Godot;

public class WRL : Node {
	readonly string path;
	public WRL(string path) : base() {
		this.path = path;
	}
	const uint WRL_MAGIC = 0x57324352;
	const uint WRL_VERSION = 0xb;
	public override void _Ready() {
		var file = new File();
		file.Open(path, File.ModeFlags.Read);

		if (file.Get32() != WRL_MAGIC) {
			throw new System.Exception("Wrong Magic");
		}
		if (file.Get32() != WRL_VERSION) {
			throw new System.Exception("Wrong Version");
		}

		while (file.GetLen() > file.GetPosition()) {
			var entry = WRLEntryFactory.Load(file);
			AddChild(entry);
		}

		foreach (var child in GetChildren()) {
			if (child is WRLEntry) {
				var entry = (WRLEntry)child;
				entry.WRLLoaded();
			}
		}
	}

	public void Save() {
		var file = new File();
		file.Open(path, File.ModeFlags.Write);

		file.Store32(WRL_MAGIC);
		file.Store32(WRL_VERSION);

		foreach (var child in GetChildren()) {
			if (child is WRLEntry) {
				var entry = (WRLEntry)child;
				WRLEntryFactory.Save(file, entry);
			}
		}

		file.Close();
	}
}

static class WRLEntryFactory {
	const uint OBMG_MAGIC = 0x474d424f;

	public static WRLEntry Load(File file) {
		if (file.Get32() != OBMG_MAGIC) {
			throw new System.Exception("Wrong Magic");
		}

		var type = file.GetFixedString(24);

		WRLEntry entry;

		switch (type) {
			case "cGeneralStatic":
			case "cGoldenBrick":
				entry = new ModelEntry(type);
				break;
			case "cLegoTerrain":
				entry = new Terrain();
				break;
			default:
				entry = new RawWRLEntry(type);
				break;
		}

		uint u = file.Get32();
		uint length = file.Get32();

		var position = file.GetPosition();

		entry.Load(file, u, length);

		if (file.GetPosition() != position + length) {
			GD.PrintErr("The WRLEntry for type \"", type, "\" didn't read the correct amount; correcting...");
			file.Seek(position + length);
		}

		return entry;
	}

	public static void Save(File file, WRLEntry entry) {
		file.Store32(OBMG_MAGIC);

		var type = entry.type;
		file.StoreFixedString(type, 24);

		file.Store32(entry.u);
		var byteLength = entry.byteLength;
		file.Store32(byteLength);

		var position = file.GetPosition();

		entry.Save(file);

		if (file.GetPosition() != position + byteLength) {
			GD.PrintErr("The WRLEntry for type \"", type, "\" didn't write the correct amount; correcting...");
			file.Seek(position + byteLength);
		}

	}
}

public abstract class WRLEntry : Node {
	public abstract string type { get; }
	public abstract uint u { get; }
	public abstract uint byteLength { get; }

	public abstract void Load(File file, uint u, uint length);
	public abstract void Save(File file);
	public virtual void WRLLoaded() { }
}

public abstract class CommonWRL : WRLEntry {
	public uint CommonByteLength { get { return 52; } }
	public uint Layer { get; set; }
	string BindingName { get; set; }
	public WRLEntry Binding { get; set; }
	protected uint LoadCommon(File file) {
		Layer = file.Get32();
		Name = file.GetFixedString(24);
		BindingName = file.GetFixedString(24);
		return CommonByteLength;
	}

	public override void WRLLoaded() {
		if (BindingName != "") {
			Binding = GetNodeOrNull<WRLEntry>("../" + BindingName);
			if (Binding == null) {
				GD.PrintErr("Couldn't find Binding \"", BindingName, "\" for ", Name);
				return;
			}
		}
		BindingName = null;
	}

	protected uint SaveCommon(File file) {
		file.Store32(Layer);
		file.StoreFixedString(Name, 24);
		if (Binding == null) {
			if (BindingName == null) {
				file.StoreFixedString("", 24);
			} else {
				file.StoreFixedString(BindingName, 24);
			}
		} else {
			file.StoreFixedString(Binding.Name, 24);
		}
		return CommonByteLength;
	}
}

class RawWRLEntry : CommonWRL {
	public override string type { get; }
	private uint _u;
	public override uint u { get { return _u; } }
	public override uint byteLength { get { return (uint)data.Length + CommonByteLength; } }

	private byte[] data;

	public RawWRLEntry(string type) {
		this.type = type;
	}
	public override void Load(File file, uint u, uint length) {
		_u = u;
		length -= LoadCommon(file);
		data = file.GetBuffer((int)length);
	}

	public override void Save(File file) {
		SaveCommon(file);
		file.StoreBuffer(data);
	}
}