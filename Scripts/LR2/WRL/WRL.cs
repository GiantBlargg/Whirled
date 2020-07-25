using Godot;
using System;
using System.Collections.Generic;

public class WRL {
	const uint WRL_MAGIC = 0x57324352;
	const uint WRL_VERSION = 0xb;
	const uint OBMG_MAGIC = 0x474d424f;

	private List<WRLEntry> entries = new List<WRLEntry>();

	public void Clear() {
		entries = new List<WRLEntry>();
	}

	public void Load(string path) {
		var file = new File();
		file.Open(path, File.ModeFlags.Read);

		if (file.Get32() != WRL_MAGIC) {
			throw new Exception("Wrong WRL Magic");
		}
		if (file.Get32() != WRL_VERSION) {
			throw new Exception("Wrong Version");
		}

		var entries = new List<WRLEntry>();

		while (file.GetLen() > file.GetPosition()) {
			if (file.Get32() != OBMG_MAGIC) {
				throw new Exception("Wrong OBGM Magic");
			}

			var type = file.GetFixedString(24);

			WRLEntry entry;

			switch (type) {
				case "cGeneralStatic":
					entry = new cGeneralStatic();
					break;
				case "cGoldenBrick":
					entry = new cGoldenBrick();
					break;
				default:
					entry = new RawWRLEntry();
					break;
			}

			entry.Type = type;
			if (entry.Type != type) {
				GD.PrintErr($"Type {type} not accepted.");
			}

			var u = file.Get32();
			entry.U = u;
			if (entry.U != u) {
				GD.PrintErr($"U {u} not accepted by type {type}.");
			}

			var length = file.Get32();
			entry.Length = length;
			if (entry.Length != length) {
				GD.PrintErr($"Length {length} not accepted by type {type}.");
			}

			var endPosition = file.GetPosition() + length;

			entry.Layer = file.Get32();
			entry.Name = file.GetFixedString(24);
			entry.Binding = file.GetFixedString(24);

			entry.Load(file);

			entries.Add(entry);

			if (file.GetPosition() != endPosition) {
				GD.PrintErr("The WRLEntry for type \"", type, "\" didn't read the correct amount; correcting...");
				file.Seek(endPosition);
			}
		}

		this.entries = entries;
	}

	public void Save(string path) {
		var file = new File();
		file.Open(path, File.ModeFlags.Write);

		file.Store32(WRL_MAGIC);
		file.Store32(WRL_VERSION);

		foreach (var entry in entries) {
			file.Store32(OBMG_MAGIC);
			var type = entry.Type;
			file.StoreFixedString(type, 24);
			file.Store32(entry.U);
			var length = entry.Length;
			file.Store32(length);

			var endPosition = file.GetPosition() + length;

			file.Store32(entry.Layer);
			file.StoreFixedString(entry.Name, 24);
			file.StoreFixedString(entry.Binding, 24);

			entry.Save(file);

			if (file.GetPosition() != endPosition) {
				GD.PrintErr("The WRLEntry for type \"", type, "\" didn't write the correct amount; correcting...");
				file.Seek(endPosition);
			}
		}

		file.Close();
	}
}

public abstract class WRLEntry {
	public virtual string Type { get; set; }
	public virtual uint U { get; set; }
	public virtual uint Length { get; set; }
	public virtual uint Layer { get; set; }
	public virtual string Name { get; set; }
	public virtual string Binding { get; set; }

	protected const uint CommonLength = 52;

	public abstract void Load(File file);
	public abstract void Save(File file);
}

class RawWRLEntry : WRLEntry {
	byte[] data;
	public override void Load(File file) {
		data = file.GetBuffer((int)(Length - CommonLength));
	}

	public override void Save(File file) {
		file.StoreBuffer(data);
	}
}

public class OldWRL : Node {
	readonly string path;
	public OldWRL(string path) : base() {
		this.path = path;
	}
	const uint WRL_MAGIC = 0x57324352;
	const uint WRL_VERSION = 0xb;
	const uint OBMG_MAGIC = 0x474d424f;

	[Signal]
	public delegate void Loaded(OldWRL wrl);

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
			if (child is OldWRLEntry) {
				var entry = (OldWRLEntry)child;
				entry.WRLLoaded();
			}
		}

		EmitSignal(nameof(Loaded), this);
	}

	public void Save() {
		var file = new File();
		file.Open(path, File.ModeFlags.Write);

		file.Store32(WRL_MAGIC);
		file.Store32(WRL_VERSION);

		foreach (var child in GetChildren()) {
			if (child is OldWRLEntry) {
				var entry = (OldWRLEntry)child;
				WRLEntryFactory.Save(file, entry);
			}
		}

		file.Close();
	}
}

static class WRLEntryFactory {
	const uint OBMG_MAGIC = 0x474d424f;

	public static OldWRLEntry Load(File file) {
		if (file.Get32() != OBMG_MAGIC) {
			throw new System.Exception("Wrong Magic");
		}

		var type = file.GetFixedString(24);

		OldWRLEntry entry;

		switch (type) {
			case "cGeneralStatic":
			case "cGoldenBrick":
				entry = new OldModelEntry(type);
				break;
			case "cLegoTerrain":
				entry = new Terrain();
				break;
			default:
				entry = new OldRawWRLEntry(type);
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

	public static void Save(File file, OldWRLEntry entry) {
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

public abstract class OldWRLEntry : Node {
	public abstract string type { get; }
	public abstract uint u { get; }
	public abstract uint byteLength { get; }
	public virtual GizmoTarget GizmoTarget { get { return null; } }
	public virtual Control PropertyControl { get { return _PropertyControl; } }
	static Control _PropertyControl = new Control();

	public abstract void Load(File file, uint u, uint length);
	public abstract void Save(File file);
	public virtual void WRLLoaded() { }
}

public abstract class CommonWRL : OldWRLEntry {
	public uint CommonByteLength { get { return 52; } }
	public uint Layer { get; set; }
	string BindingName { get; set; }
	public OldWRLEntry Binding { get; set; }
	protected uint LoadCommon(File file) {
		Layer = file.Get32();
		Name = file.GetFixedString(24);
		BindingName = file.GetFixedString(24);
		return CommonByteLength;
	}

	public override void WRLLoaded() {
		if (BindingName != "") {
			Binding = GetNodeOrNull<OldWRLEntry>("../" + BindingName);
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

class OldRawWRLEntry : CommonWRL {
	public override string type { get; }
	private uint _u;
	public override uint u { get { return _u; } }
	public override uint byteLength { get { return (uint)data.Length + CommonByteLength; } }

	private byte[] data;

	public OldRawWRLEntry(string type) {
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