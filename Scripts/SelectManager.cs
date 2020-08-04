using Godot;
using System.Collections.Generic;
using Controls;

public class SelectManager : Node {
	[Export]
	public NodePath WrlManager;
	WRLManager wrl;
	[Export]
	public NodePath WRLTreeView;
	Tree tree;
	[Export]
	public NodePath PropertyContainer;
	ScrollContainer propContainer;

	Control currentControl;

	Dictionary<string, TreeItem> nameLookup;

	public override void _Ready() {
		wrl = GetNode<WRLManager>(WrlManager);
		tree = GetNode<Tree>(WRLTreeView);
		propContainer = GetNode<ScrollContainer>(PropertyContainer);

		wrl.Connect(nameof(WRLManager.Loaded), this, nameof(PopulateTree));
		tree.Connect("item_selected", this, nameof(DisplayControls));
		wrl.Connect(nameof(WRLManager.PropertySet), this, nameof(UpdateControls));
	}

	void PopulateSubTree(TreeItem root, List<NameTreeElem> nameTree) {
		foreach (var elem in nameTree) {
			var child = tree.CreateItem(root);
			child.SetText(0, elem.Name);
			PopulateSubTree(child, elem.Children);
			child.Collapsed = true;
			nameLookup[elem.Name] = child;
		}
	}

	public void PopulateTree() {
		tree.Clear();
		nameLookup = new Dictionary<string, TreeItem>();

		var nameTree = wrl.NameTree;

		var root = tree.CreateItem();

		PopulateSubTree(root, nameTree);
	}

	Dictionary<string, List<IControl>> namedControls;

	public void UpdateControls(object value, string name, string prop) {
		if (name != tree.GetSelected().GetText(0)) return;
		var c = namedControls[prop];
		if (c == null) return;

		c.ForEach(con => con.Update(value));
	}

	public void Select(string name) {
		var item = nameLookup[name];
		item.Select(0);
		tree.EnsureCursorIsVisible();
	}

	public void DisplayControls() {
		if (namedControls != null)
			foreach (var c in namedControls) {
				c.Value.ForEach(o => ((Node)o).QueueFree());
			}
		if (currentControl != null)
			currentControl.QueueFree();

		currentControl = new HBoxContainer();
		currentControl.SizeFlagsHorizontal = (int)Control.SizeFlags.ExpandFill;
		namedControls = new Dictionary<string, List<IControl>>();

		var name = tree.GetSelected().GetText(0);
		var props = wrl.GetProperties(name);

		foreach (var prop in props) {
			args = new string[] { name, prop.name };
			if (prop.type == typeof(Transform)) {
				AddControl(new TransformControl(prop.flags.HasFlag(LR2.WRL.PropertyFlags.Scale)));
				AddControl(Gizmo.scene.Instance(), wrl.ResolveParent(name));
			} else {
				GD.PrintErr($"Unkown type {prop.type} on member {prop.name}");
				continue;
			}
		}
		args = null;

		propContainer.AddChild(currentControl);
	}

	string[] args = null;

	void AddControl(Node control, Node mountPoint = null) {
		if (mountPoint == null)
			mountPoint = currentControl;

		control.Connect("ValueSet", wrl, nameof(WRLManager.SetProperty), new Godot.Collections.Array(args));
		mountPoint.AddChild(control);
		if (control is IControl) {
			(control as IControl).Update(wrl.GetProperty(args[0], args[1]));
			if (!namedControls.ContainsKey(args[1]))
				namedControls[args[1]] = new List<IControl>();
			namedControls[args[1]].Add(control as IControl);
		} else {
			GD.PrintErr("Not IControl");
		}
	}
}

interface IControl {
	void Update(object value);
}
