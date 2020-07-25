using Godot;

public class WRLManager : Node {
	WRL wrl = new WRL();
	string path;

	public void New() {
		path = null;
		wrl.Clear();
	}

	public void Open(string _path) {
		New();
		path = _path;
		wrl.Load(path);
	}

	public void Save() {
		Save(null);
	}

	public void Save(string _path = null) {
		if (_path != null)
			path = _path;
		wrl.Save(path);
	}
}