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
		}
	}

	public void PopulateTree() {
		tree.Clear();

		var nameTree = wrl.NameTree;

		var root = tree.CreateItem();

		PopulateSubTree(root, nameTree);
	}

	Dictionary<string, IControl> namedControls;

	public void UpdateControls(object value, string name, string prop) {
		if (name != tree.GetSelected().GetText(0)) return;
		var c = namedControls[prop];
		if (c == null) return;
		c.Update(value);
	}

	public void DisplayControls() {
		if (currentControl != null)
			currentControl.QueueFree();

		currentControl = new HBoxContainer();
		currentControl.SizeFlagsHorizontal = (int)Control.SizeFlags.ExpandFill;
		namedControls = new Dictionary<string, IControl>();

		var name = tree.GetSelected().GetText(0);
		var props = wrl.GetProperties(name);
		foreach (var prop in props) {
			Control control;
			if (prop.type == typeof(Transform)) {
				control = new TransformControl(prop.flags.HasFlag(LR2.WRL.PropertyFlags.Scale));
			} else {
				GD.PrintErr($"Unkown type {prop.type} on member {prop.name}");
				continue;
			}
			var args = new Godot.Collections.Array(new string[] { name, prop.name });
			control.Connect("ValueSet", wrl, nameof(WRLManager.SetProperty), args);
			currentControl.AddChild(control);
			if (control is IControl) {
				(control as IControl).Update(wrl.GetProperty(name, prop.name));
				namedControls[prop.name] = control as IControl;
			} else {
				GD.PrintErr("Not IControl");
			}
		}

		propContainer.AddChild(currentControl);
	}
}

interface IControl {
	void Update(object value);
}