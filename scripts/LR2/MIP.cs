using Godot;

public static class MIP {

	public static Texture LoadTexture(string path, GameDataManager gameDataManager) {
		switch (path.Substring(path.Length - 3).ToLower()) {
			case "tga": {
					var resolvedPath = gameDataManager.ResolvePath(path.Remove(path.Length - 3) + "mip");
					return LoadMIP(resolvedPath);
				}
			case "ifl": {
					var resolvedPath = gameDataManager.ResolvePath(path);
					return LoadIFL(resolvedPath);
				}
			case "mip": {
					GD.PrintErr("Huh... Okay: ", path);
					var resolvedPath = gameDataManager.ResolvePath(path);
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
		File f = new File();
		f.Open(path, File.ModeFlags.Read);

		var IDLength = f.Get8();
		var ColourMapType = f.Get8();
		var ImageType = f.Get8();

		if (ImageType != 1 && ImageType != 2) {
			GD.PrintErr("Only non-compressed textures are supported: ", path);
			return Garbage();
		}

		var ColourMapIndex = f.Get16();
		var ColourMapLength = f.Get16();
		var ColourDepth = f.Get8();

		if (ImageType == 1 && ColourDepth != 32) {
			GD.PrintErr("Only 32-bit colour is supported: ", path);
			return Garbage();
		}

		{
			var x = f.Get16();
			var y = f.Get16();
			if (x != 0 || y != 0) {
				GD.PrintErr("Origin must be (0,0): ", path);
				return Garbage();
			}
		}

		var width = f.Get16();
		var height = f.Get16();
		var pixelDepth = f.Get8();
		var imageDescriptor = f.Get8();

		if (ImageType == 1 && pixelDepth != 8) {
			GD.PrintErr("Pixel depth must be 8 for colour mapped textures: ", path);
			return Garbage();
		}

		if (ImageType == 2 && pixelDepth != 32 && pixelDepth != 24) {
			GD.PrintErr("Pixel depth must be 32 or 24 for non colour mapped textures: ", path);
			return Garbage();
		}

		if (imageDescriptor != 0) {
			GD.PrintErr("Unsupported imageDescriptor: ", path);
			return Garbage();
		}

		f.Seek(f.GetPosition() + IDLength);

		var ColourMap = new Color[ColourMapLength];

		if (ColourMapType == 1) {
			for (int i = ColourMapIndex; i < ColourMapLength; i++) {
				ColourMap[i] = f.GetColorBGRA8();
			}
		}

		var image = new Image();

		if (ImageType == 1) {
			image.Create(width, height, false, Image.Format.Rgba8);
			image.Lock();

			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					image.SetPixel(x, y, ColourMap[f.Get8()]);
				}
			}

		} else if (ImageType == 2) {
			if (pixelDepth == 24) {
				image.Create(width, height, false, Image.Format.Rgb8);
			} else if (pixelDepth == 32) {
				image.Create(width, height, false, Image.Format.Rgba8);
			}
			image.Lock();

			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					if (pixelDepth == 24) {
						image.SetPixel(x, y, f.GetColorBGR8());
					} else if (pixelDepth == 32) {
						image.SetPixel(x, y, f.GetColorBGRA8());
					}
				}
			}
		}

		image.Unlock();
		image.SavePng(path + ".png");
		var texture = new ImageTexture();
		texture.CreateFromImage(image);
		texture.Flags = 6;
		return texture;
	}

	const float delayTime = 1f / 30f;

	static Texture LoadIFL(string path) {
		var regex = new RegEx();
		regex.Compile("^(.*[\\/\\\\]).*$");
		var result = regex.Search(path);
		var dirPath = result.GetString(1);
		regex.Compile("(\\S+)\\s(\\d+)");

		var f = new File();
		f.Open(path, File.ModeFlags.Read);
		var contents = f.GetAsText();
		f.Close();

		var texture = new AnimatedTexture();

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