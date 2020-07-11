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
}
