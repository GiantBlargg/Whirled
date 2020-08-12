using Godot;
using System.Collections.Generic;
using LR2.WRL;

public class WRLManager : Node {
	WRL wrl = new WRL();
	string path;

	Node root;

	[Export]
	public NodePath rootMount;

	[Signal]
	public delegate void Loaded();

	[Signal]
	public delegate void Clicked(string name);

	public void Input(object camera, InputEvent ev, Vector3 click_position, Vector3 click_normal, int shape_idx, string name) {
		if (ev.IsPressed()) {
			EmitSignal(nameof(Clicked), name);
		}
	}

	public Node ResolveParent(string name) => ResolveParent(wrl.nameLookup[name]);
	Node ResolveParent(WRLEntry entry) {
		if (entry.Binding != null && entry.Binding.Node != null)
			return entry.Binding.Node;
		return root;
	}

	public override void _Ready() {
		root = GetNode<Spatial>(rootMount);
		wrl.EntryAdded = (entry) => {
			//TODO: mount under binding's mount
			var node = entry.Node;
			if (node != null)
				ResolveParent(entry).AddChild(node);

			var collider = entry.selectCollider;
			if (collider != null) {
				collider.Connect("input_event", this, nameof(Input), new Godot.Collections.Array(new string[] { entry.Name }));
			}
		};
	}

	void Clear() {
		wrl.Clear();
		foreach (Node child in GetNode(rootMount).GetChildren()) {
			child.QueueFree();
		}
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

	public List<ObjectProperty> GetProperties(string name) => wrl.GetProperties(name);

	[System.Obsolete]
	public object GetProperty(string name, string prop) => wrl.GetProperty(name, prop);
}

public struct NameTreeElem {
	public string Name;
	public List<NameTreeElem> Children;
}