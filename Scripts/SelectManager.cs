using Godot;
using System;
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
			foreach (var c in namedControls.Values) {
				c.ForEach(o => o.QueueFree());
			}
		if (currentControl != null)
			currentControl.QueueFree();

		currentControl = new VBoxContainer();
		currentControl.SizeFlagsHorizontal = (int)Control.SizeFlags.ExpandFill;
		namedControls = new Dictionary<string, List<IControl>>();

		var name = tree.GetSelected().GetText(0);
		var props = wrl.GetProperties(name);

		foreach (var prop in props) {
			args = new string[] { name, prop.name };
			if (prop.type == typeof(Transform)) {
				AddControl(new TransformControl(prop.flags.HasFlag(LR2.WRL.PropertyFlags.Scale)));
				AddControl(Gizmo.scene.Instance(), wrl.ResolveParent(name));
			} else if (typeof(IConvertible).IsAssignableFrom(prop.type)) {
				AddControl(new Number(prop.type));
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
		var nodeName = args[0];
		var paramName = args[1];
		if (mountPoint == null)
			mountPoint = currentControl;

		mountPoint.AddChild(control);
		control.Name = paramName;
		if (control is IControl) {
			var c = control as IControl;
			c.ValueSet += value => wrl.SetProperty(value, nodeName, paramName);
			c.Update(wrl.GetProperty(nodeName, paramName));
			if (!namedControls.ContainsKey(paramName))
				namedControls[paramName] = new List<IControl>();
			namedControls[paramName].Add(c);
		} else {
			GD.PrintErr("Not IControl");
		}
	}
}

namespace Controls {

	public delegate void ValueSet(object value);

	interface IControl {
		void Update(object value);
		void QueueFree();
		event ValueSet ValueSet;
	}
}
