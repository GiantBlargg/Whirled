using Godot;

public static class MIP {

	public static Texture LoadTexture(string path, GameDataManager gameDataManager) {
		switch (path.Substring(path.Length - 3).ToLower()) {
			case "tga": {
					var resolvedPath = gameDataManager.ResolvePath(path.Remove(path.Length - 3) + "mip");
					if (resolvedPath == null) return null;
					return LoadMIP(resolvedPath);
				}
			case "ifl": {
					var resolvedPath = gameDataManager.ResolvePath(path);
					if (resolvedPath == null) return null;
					return LoadIFL(resolvedPath);
				}
			case "mip": {
					GD.PrintErr("Huh... Okay: ", path);
					var resolvedPath = gameDataManager.ResolvePath(path);
					if (resolvedPath == null) return null;
					return LoadMIP(resolvedPath);
				}
			default: {
					GD.PrintErr("Unsupported Texture Type: ", path);
					return Garbage();
				}
		}
	}

	static Texture Garbage() {
		var texture = new GradientTexture();
		var g = new Gradient();
		g.Colors = new Color[] { new Color(1, 0, 0), new Color(0, 1, 0) };
		texture.Gradient = g;
		return texture;
	}

	static Color GetColorBGRA8(this File file) {
		var color = new Color();
		color.b8 = file.Get8();
		color.g8 = file.Get8();
		color.r8 = file.Get8();
		color.a8 = file.Get8();
		return color;
	}

	static Color GetColorBGR8(this File file) {
		var color = new Color();
		color.b8 = file.Get8();
		color.g8 = file.Get8();
		color.r8 = file.Get8();
		return color;
	}

	static Texture LoadMIP(string path) {
		var image = new Image();
		image.Load(path);
		image.FlipY();

		var texture = new ImageTexture();
		texture.CreateFromImage(image, 31);
		return texture;
	}

	const float delayTime = 1f / 30f;

	static Texture LoadIFL(string path) {
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