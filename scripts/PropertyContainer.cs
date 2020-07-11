using Godot;

public class PropertyContainer : ScrollContainer {
	[Export]
	public NodePath WRLItemListPath;
	ItemList WRLItemList;

	Control currentProps;

	public override void _Ready() {
		WRLItemList = (ItemList)GetNode(WRLItemListPath);
		WRLItemList.Connect("multi_selected", this, nameof(UpdateProps));
	}

	public void UpdateProps(int _, bool __) {
		if (currentProps != null)
			RemoveChild(currentProps);

		var selected = WRLItemList.GetSelectedItems();
		if (selected.Length == 1) {
			currentProps = ((WRLEntry)GetNode("/root/GameDataManager/WRL/" + WRLItemList.GetItemText(selected[0]))).PropertyControl;
			if (currentProps != null)
				AddChild(currentProps);
		}
	}
}
