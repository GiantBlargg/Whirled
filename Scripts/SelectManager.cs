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

	List<IControl> controls = new List<IControl>();

	public void Select(string name) {
		var item = nameLookup[name];
		item.Select(0);
		tree.EnsureCursorIsVisible();
	}

	public void DisplayControls() {
		if (controls != null) controls.ForEach(c => c.QueueFree());
		controls.Clear();

		if (currentControl != null)
			currentControl.QueueFree();

		currentControl = new VBoxContainer();
		currentControl.SizeFlagsHorizontal = (int)Control.SizeFlags.ExpandFill;

		var name = tree.GetSelected().GetText(0);
		var props = wrl.GetProperties(name);

		var label = new Label();
		label.Text = (string)wrl.GetProperty(name, "Type");
		currentControl.AddChild(label);

		foreach (var prop in props) {
			if (prop is ObjectProperty<Transform>) {
				AddControl(prop as ObjectProperty<Transform>, new TransformControl(/*prop.flags.HasFlag(LR2.WRL.PropertyFlags.Scale)*/));
				AddControl(prop as ObjectProperty<Transform>, Gizmo.scene.Instance() as Gizmo, wrl.ResolveParent(name));
			} else if (prop is ObjectProperty<string>) {
				AddControl(prop as ObjectProperty<string>, new StringControl());
			} else if (prop is ObjectProperty<int>) {
				AddControl(prop as ObjectProperty<int>, new Number<int>());
			} else if (prop is ObjectProperty<uint>) {
				AddControl(prop as ObjectProperty<uint>, new Number<uint>());
			} else if (prop is ObjectProperty<float>) {
				AddControl(prop as ObjectProperty<float>, new Number<float>());
			} else {
				GD.PrintErr($"Unkown type {prop.Type} on member {prop.Name}");
				continue;
			}

		}

		propContainer.AddChild(currentControl);
	}

	void AddControl<T>(ObjectProperty<T> property, IControl<T> control, Node mountPoint = null) {
		if (mountPoint == null)
			mountPoint = currentControl;

		if (control is Node)
			mountPoint.AddChild(control as Node);

		control.Name = property.Name;

		control.ValueSet += property.Set;
		property.Update += control.Update;
		control.Update(property.Value);
		controls.Add(control);
	}
}

namespace Controls {

	public delegate void ValueSet<T>(T value, bool updateHistory = true);

	public interface IControl {
		string Name { get; set; }
		void QueueFree();
	}
	public interface IControl<T> : IControl {
		void Update(T value);
		event ValueSet<T> ValueSet;
	}
}
