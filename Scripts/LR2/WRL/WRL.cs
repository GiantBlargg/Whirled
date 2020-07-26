using Godot;
using System;
using System.Reflection;
using System.Collections.Generic;
using System.Linq;

namespace LR2.WRL {

	public class WRL {
		const uint WRL_MAGIC = 0x57324352;
		const uint WRL_VERSION = 0xb;
		const uint OBMG_MAGIC = 0x474d424f;

		private List<WRLEntry> entries = new List<WRLEntry>();
		private Dictionary<string, WRLEntry> nameLookup = new Dictionary<string, WRLEntry>();

		public Spatial rootMount;

		public void Clear() {
			entries = new List<WRLEntry>();
			nameLookup = new Dictionary<string, WRLEntry>();
			foreach (Node child in rootMount.GetChildren()) {
				child.QueueFree();
			}
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
			var nameLookup = new Dictionary<string, WRLEntry>();

			var bindings = new List<(WRLEntry entry, string binding)>();

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
					case "cLegoTerrain":
						entry = new cLegoTerrain();
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
				var binding = file.GetFixedString(24);

				entry.Load(file);

				if (binding == "") {
					entries.Add(entry);
				} else {
					bindings.Add((entry, binding));
				}
				nameLookup.Add(entry.Name, entry);

				{//TODO: mount under binding's mount
					var node = entry.Node;
					if (node != null)
						rootMount.AddChild(entry.Node);
				}

				if (file.GetPosition() != endPosition) {
					GD.PrintErr("The WRLEntry for type \"", type, "\" didn't read the correct amount; correcting...");
					file.Seek(endPosition);
				}
			}

			foreach (var b in bindings) {
				var binding = nameLookup[b.binding];
				binding.children.Add(b.entry);
				b.entry.Binding = binding;
			}

			this.entries = entries;
			this.nameLookup = nameLookup;
		}

		static void SaveEntry(File file, WRLEntry entry) {
			file.Store32(OBMG_MAGIC);
			var type = entry.Type;
			file.StoreFixedString(type, 24);
			file.Store32(entry.U);
			var length = entry.Length;
			file.Store32(length);

			var endPosition = file.GetPosition() + length;

			file.Store32(entry.Layer);
			file.StoreFixedString(entry.Name, 24);

			var binding = "";
			if (entry.Binding != null)
				binding = entry.Binding.Name;
			file.StoreFixedString(binding, 24);

			entry.Save(file);

			if (file.GetPosition() != endPosition) {
				GD.PrintErr("The WRLEntry for type \"", type, "\" didn't write the correct amount; correcting...");
				file.Seek(endPosition);
			}

			foreach (var e in entry.children) {
				SaveEntry(file, e);
			}
		}

		public void Save(string path) {
			var file = new File();
			file.Open(path, File.ModeFlags.Write);

			file.Store32(WRL_MAGIC);
			file.Store32(WRL_VERSION);

			foreach (var entry in entries) {
				SaveEntry(file, entry);
			}

			file.Close();
		}

		static List<NameTreeElem> NameTreePart(List<WRLEntry> entries) =>
		entries.Select(entry =>
			new NameTreeElem() {
				Name = entry.Name,
				Children = NameTreePart(entry.children)
			}).ToList();

		public List<NameTreeElem> NameTree => NameTreePart(entries);

		public List<PropertyType> GetProperties(string name) =>
			nameLookup[name].GetType().GetProperties()
				.Where(prop => Attribute.IsDefined(prop, typeof(WRLPropertyAttribute)))
				.Select(prop => {
					var flags = (prop.GetCustomAttribute(typeof(WRLPropertyAttribute)) as WRLPropertyAttribute).flags;
					return new PropertyType() {
						name = prop.Name,
						type = prop.PropertyType,
						flags = flags
					};
				}).ToList();

		public void SetProperty(object value, string name, string prop) {
			var target = nameLookup[name];
			target.GetType().GetProperty(prop).SetValue(target, value);
		}

		public object GetProperty(string name, string propName) {
			var target = nameLookup[name];
			var prop = target.GetType().GetProperty(propName);
			return prop.GetValue(target);
		}
	}

	[Flags]
	public enum PropertyFlags {
		None = 0x0,
		Safe = 0x1,
		Scale = 0x2,
	}

	public struct PropertyType {
		public string name;
		public Type type;
		public PropertyFlags flags;
	}

	[AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
	public class WRLPropertyAttribute : Attribute {
		public PropertyFlags flags;
	}

	public abstract class WRLEntry {
		public virtual string Type { get; set; }
		public virtual uint U { get; set; }
		public virtual uint Length { get; set; }
		public uint Layer;
		public string Name;
		public WRLEntry Binding;
		public List<WRLEntry> children = new List<WRLEntry>();

		public virtual Node Node => null;
		public virtual Node Mount => Node;

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
