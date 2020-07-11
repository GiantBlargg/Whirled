using Godot;
using System;

public class WRLEntryList : ItemList {
	public void onWRLLoaded(WRL wrl) {
		foreach (var child in wrl.GetChildren()) {
			if (child is WRLEntry) {
				var entry = (WRLEntry)child;
				AddItem(entry.Name);
			}
		}
	}
	public delegate void ItemSelected(string name, bool selected);
	public void _ItemSelected(int i, bool selected) {
		EmitSignal(nameof(ItemSelected), GetItemText(i), selected);
	}
}
