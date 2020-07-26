using Godot;
using System.Collections.Generic;
using LR2.WRL;

public class WRLManager : Node {
	WRL wrl = new WRL();
	string path;

	[Export]
	public NodePath rootMount;

	[Signal]
	public delegate void Loaded();

	void Clear() {
		wrl.rootMount = GetNode<Spatial>(rootMount);
		wrl.Clear();
	}

	public void New() {
		path = null;
		Clear();
		EmitSignal(nameof(Loaded));
	}

	public void Open(string _path) {
		Clear();
		path = _path;
		wrl.Load(path);
		EmitSignal(nameof(Loaded));
	}

	public void Save() {
		Save(null);
	}

	public void Save(string _path = null) {
		if (_path != null)
			path = _path;
		wrl.Save(path);
	}

	public List<NameTreeElem> NameTree => wrl.NameTree;

	public List<PropertyType> GetProperties(string name) => wrl.GetProperties(name);

	[Signal]
	public delegate void PropertySet(Godot.Object value, string name, string prop);

	public void SetProperty(object value, string name, string prop) {
		wrl.SetProperty(value, name, prop);
		EmitSignal(nameof(PropertySet), value, name, prop);
	}

	public object GetProperty(string name, string prop) => wrl.GetProperty(name, prop);
}

public struct NameTreeElem {
	public string Name;
	public List<NameTreeElem> Children;
}