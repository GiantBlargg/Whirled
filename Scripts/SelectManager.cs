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

		wrl.Connect(nameof(WRLManager.Loaded), new Callable(this, nameof(PopulateTree)));
		tree.ItemSelected += DisplayControls;
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
		if (controls != null) controls.ForEach(c => c.QueueFree());
		controls.Clear();

		if (currentControl != null)
			currentControl.QueueFree();

		tree.Clear();
		nameLookup = new Dictionary<string, TreeItem>();

		var nameTree = wrl.NameTree;

		var root = tree.CreateItem();

		PopulateSubTree(root, nameTree);
	}

	List<IControl> controls = new List<IControl>();

	public void Select(string name) {
		var item = nameLookup[name];
		for (var i = item.GetParent(); i.GetParent() != null; i = i.GetParent()) {
			i.Collapsed = false;
		}
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
			switch (prop) {
				case ObjectProperty<Transform> p:
					AddControl(p, new TransformControl(/*prop.flags.HasFlag(LR2.WRL.PropertyFlags.Scale)*/));
					AddControl(p, Gizmo.scene.Instance() as Gizmo, wrl.ResolveParent(name));
					break;
				case ObjectProperty<string> p:
					AddControl(p, new StringControl());
					break;
				case ObjectProperty<int> p:
					AddControl(p, new Number<int>());
					break;
				case ObjectProperty<uint> p:
					AddControl(p, new Number<uint>());
					break;
				case ObjectProperty<float> p:
					AddControl(p, new Number<float>());
					break;
				case ObjectProperty<Vector2> p:
					AddControl(p, new Vector2Control());
					break;
				case ObjectProperty<Vector3> p:
					AddControl(p, new Vector3Control());
					break;
				case ObjectProperty<byte[]> p:
					AddControl(p, new ArrayControl<byte>(p.Get().Length, () => new Number<byte>()));
					break;
				case ObjectProperty<uint[]> p:
					AddControl(p, new ArrayControl<uint>(p.Get().Length, () => new Number<uint>()));
					break;
				case ObjectProperty<Vector3[]> p:
					AddControl(p, new ArrayControl<Vector3>(p.Get().Length, () => new Vector3Control()));
					break;
				default:
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

		control.ValueSet += h => property.Set(control.Value, h);
		property.Update += v => control.Value = v;
		control.Value = property.Value;
		controls.Add(control);
	}
}

namespace Controls {

	public delegate void ValueSet(bool updateHistory = true);

	public interface IControl {
		StringName Name { get; set; }
		void QueueFree();
		event ValueSet ValueSet;
	}
	public interface IControl<T> : IControl {
		T Value { get; set; }
	}
}
