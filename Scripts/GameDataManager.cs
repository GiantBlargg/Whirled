using Godot;

public class GameDataManager : Node {
	ConfigFile config;
	const string configPath = "user://settings.cfg";
	public override void _Ready() {
		config = new ConfigFile();
		config.Load(configPath);
		GD.Print(config.GetValue(section, key, ""));
		EmitSignal(nameof(DataPathSet), config.GetValue(section, key, ""));
	}

	private const string section = "", key = "GamePath";

	[Signal]
	public delegate void DataPathSet(string path);

	public string dataPath {
		get {
			return (string)config.GetValue(section, key, "");
		}
		set {
			config.SetValue(section, key, value);
			config.Save(configPath);
		}
	}

	public void DataPathSelected(string path) {
		dataPath = path;
		EmitSignal(nameof(DataPathSet), path);
	}

	public string ResolvePath(string path) {
		return ResolvePathStatic(dataPath, path);
	}

	//This function corrects the case sensitivity of filenames, not needed on windows but I couldn't be bothered to disable it
	public static string ResolvePathStatic(string dataPath, string path) {
		Directory currentDir = new Directory();
		currentDir.Open(dataPath);

		string next = "";

		foreach (var pathDir in path.Split(new char[] { '/', '\\' })) {
			currentDir.ListDirBegin(true, true);
			do {
				next = currentDir.GetNext();
				if (next == "") {
					GD.PrintErr("Unable to resolve \"" + path + "\" with dataPath \"" + dataPath + "\"");
					return null;
				}
			} while (next.ToLower() != pathDir.ToLower());
			currentDir.ListDirEnd();
			currentDir.ChangeDir(next);
		}
		return currentDir.GetCurrentDir() + "/" + next;
	}

	public void DeduceDataPath(string path) {
		if (dataPath == "") {
			var regex = new RegEx();
			regex.Compile("^(.*[\\/\\\\]).*[\\/\\\\].*[\\/\\\\].*$");
			var result = regex.Search(path);
			var gamePath = result.GetString(1);
			dataPath = gamePath;
		}
	}
}
