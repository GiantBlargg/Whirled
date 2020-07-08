using Godot;

public static class MIP {

	public static Texture2D LoadTexture(string path, GameDataManager gameDataManager) {
		switch (path.Substring(path.Length - 3).ToLower()) {
			case "tga": {
					var resolvedPath = gameDataManager.ResolvePath(path.Remove(path.Length - 3) + "mip");
					return LoadMIP(resolvedPath);
				}
			case "ifl": {
					GD.PrintErr("Animated Textures are not supported");
					return Garbage();
				}
			case "mip": {
					GD.PrintErr("Huh... Okay");
					var resolvedPath = gameDataManager.ResolvePath(path);
					return LoadMIP(resolvedPath);
				}
			default: {
					GD.PrintErr("Unsupported Texture Type");
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

	static string ResolveTGAPath(string path, GameDataManager gameDataManager) {
		return gameDataManager.ResolvePath(path.Remove(path.Length - 3) + "mip");
	}

	static Color GetColor8(this File file) {
		var color = new Color();
		color.b8 = file.Get8();
		color.g8 = file.Get8();
		color.r8 = file.Get8();
		color.a8 = file.Get8();
		// return new Color(file.Get32());
		return color;
	}

	static Texture2D LoadMIP(string path) {
		File f = new File();
		f.Open(path, File.ModeFlags.Read);

		var IDLength = f.Get8();
		var ColourMapType = f.Get8();
		var ImageType = f.Get8();

		if (ImageType != 1 && ImageType != 2) {
			GD.PrintErr("Only non-compressed textures are supported");
			return Garbage();
		}

		var ColourMapIndex = f.Get16();
		var ColourMapLength = f.Get16();
		var ColourDepth = f.Get8();

		if (ImageType == 1 && ColourDepth != 32) {
			GD.PrintErr("Only 32-bit colour is supported");
			return Garbage();
		}

		{
			var x = f.Get16();
			var y = f.Get16();
			if (x != 0 || y != 0) {
				GD.PrintErr("Origin must be (0,0)");
				return Garbage();
			}
		}

		var width = f.Get16();
		var height = f.Get16();
		var pixelDepth = f.Get8();
		var imageDescriptor = f.Get8();

		if (ImageType == 1 && pixelDepth != 8) {
			GD.PrintErr("Pixel depth must be 8 for colour mapped textures");
			return Garbage();
		}

		if (ImageType == 2 && pixelDepth != 32) {
			GD.PrintErr("Pixel depth must be 32 for non colour mapped textures");
			return Garbage();
		}

		if (imageDescriptor != 0) {
			GD.PrintErr("Unsupported imageDescriptor");
			return Garbage();
		}

		f.Seek(f.GetPosition() + IDLength);

		var ColourMap = new Color[ColourMapLength];

		if (ColourMapType == 1) {
			for (int i = ColourMapIndex; i < ColourMapLength; i++) {
				ColourMap[i] = f.GetColor8();
			}
		}

		var image = new Image();

		if (ImageType == 1) {
			image.Create(width, height, false, Image.Format.Rgba8);

			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					image.SetPixel(x, y, ColourMap[f.Get8()]);
				}
			}

		} else if (ImageType == 2) {
			image.Create(width, height, false, Image.Format.Rgba8);
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					image.SetPixel(x, y, f.GetColor8());
				}
			}
		}

		var texture = new ImageTexture();
		texture.CreateFromImage(image);
		return texture;
	}
}