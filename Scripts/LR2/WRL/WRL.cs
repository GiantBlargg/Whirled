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

		public List<WRLEntry> entries = new List<WRLEntry>();
		public Dictionary<string, WRLEntry> nameLookup = new Dictionary<string, WRLEntry>();

		public delegate void EntryAddedFunc(WRLEntry entry);
		public EntryAddedFunc EntryAdded;

		public void Clear() {
			entries = new List<WRLEntry>();
			nameLookup = new Dictionary<string, WRLEntry>();
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

				if (file.GetPosition() != endPosition) {
					GD.PrintErr("The WRLEntry for type \"", type, "\" didn't read the correct amount; correcting...");
					file.Seek(endPosition);
				}

				EntryAdded(entry);

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

		public List<ObjectProperty> GetProperties(string name) => nameLookup[name].GetProperties();

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
		[WRLProperty]
		public uint Layer { get; set; }
		public string Name { get; set; }
		public WRLEntry Binding;
		public List<WRLEntry> children = new List<WRLEntry>();

		public virtual Node Node => null;
		public virtual Node Mount => Node;
		public virtual CollisionObject selectCollider => null;

		protected const uint CommonLength = 52;

		public abstract void Load(File file);
		public abstract void Save(File file);

		public virtual List<ObjectProperty> GetProperties() {
			var props = new List<ObjectProperty>();

			props.Add(new ObjectProperty<uint>(() => Layer));

			return props;
		}
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
