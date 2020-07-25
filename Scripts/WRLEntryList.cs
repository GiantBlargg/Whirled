using Godot;
using System;

public class WRLEntryList : ItemList {
	public void onWRLLoaded(OldWRL wrl) {
		foreach (var child in wrl.GetChildren()) {
			if (child is OldWRLEntry) {
				var entry = (OldWRLEntry)child;
				AddItem(entry.Name);
			}
		}
	}
}
