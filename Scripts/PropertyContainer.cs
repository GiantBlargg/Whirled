using Godot;

public class PropertyContainer : ScrollContainer {
	[Export]
	public NodePath WRLItemListPath;
	ItemList WRLItemList;

	Control currentProps;

	public override void _Ready() {
		WRLItemList = (ItemList)GetNode(WRLItemListPath);
		WRLItemList.Connect("multi_selected", this, nameof(onSelect));
	}

	public void onSelect(int _, bool __) { UpdateProps(); }

	public void UpdateProps() {
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
