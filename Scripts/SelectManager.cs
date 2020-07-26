using Godot;
using System.Collections.Generic;

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

	public override void _Ready() {
		wrl = GetNode<WRLManager>(WrlManager);
		tree = GetNode<Tree>(WRLTreeView);
		propContainer = GetNode<ScrollContainer>(PropertyContainer);

		wrl.Connect(nameof(WRLManager.Loaded), this, nameof(PopulateTree));
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
}