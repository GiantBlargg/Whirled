using Godot;

public static class MIP {

	public static Texture2D LoadTexture(string path) {
		switch (path.Substring(path.Length - 3).ToLower()) {
			case "mip":
			case "tga":
				return ResourceLoader.Load<ImageTexture>("lr2://" + path);
			case "ifl": {
					var resolvedPath = LR2Dir.ResolvePath(path);
					if (resolvedPath == null) return null;
					return LoadIFL(resolvedPath);
				}
			default: {
					GD.PrintErr("Unsupported Texture Type: ", path);
					return Garbage();
				}
		}
	}

	static Texture2D Garbage() {
		var texture = new GradientTexture();
		var g = new Gradient();
		g.Colors = new Color[] { new Color(1, 0, 0), new Color(0, 1, 0) };
		texture.Gradient = g;
		return texture;
	}

	static Texture2D LoadMIP(string path) {
		return ResourceLoader.Load<ImageTexture>(path);
	}

	const float delayTime = 1f / 30f;

	static Texture2D LoadIFL(string path) {
		var regex = new RegEx();
		regex.Compile("^(.*[\\/\\\\]).*$");
		var result = regex.Search(path);
		var dirPath = result.GetString(1);

		var f = new File();
		f.Open(path, File.ModeFlags.Read);
		var contents = f.GetAsText();
		f.Close();

		var texture = new AnimatedTexture();

		regex.Compile("(\\S+)\\s(\\d+)");
		var matches = regex.SearchAll(contents);
		texture.Frames = matches.Count;
		texture.Fps = 0;
		for (int i = 0; i < matches.Count; i++) {
			RegExMatch match = (RegExMatch)matches[i];

			var mipPath = (string)match.Strings[1];
			var resolvedMipPath = GameDataManager.ResolvePathStatic(dirPath, mipPath.Remove(mipPath.Length - 3) + "mip");

			var frame = LoadMIP(resolvedMipPath);

			texture.SetFrameTexture(i, frame);
			texture.SetFrameDelay(i, delayTime * int.Parse((string)match.Strings[2]));
		}

		return texture;
	}
}