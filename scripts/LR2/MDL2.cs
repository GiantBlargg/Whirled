using Godot;

public class MDL2 : MeshInstance3D {
	public string modelPath {
		get { return _modelPath; }
		set {
			_modelPath = value;
			LoadModel();
		}
	}
	private string _modelPath;
	private bool delayedLoad = false;

	public override void _EnterTree() {
		if (delayedLoad)
			LoadModel();
	}

	void LoadModel() {
		if (IsInsideTree()) {
			var gameDataManager = GetNode<GameDataManager>("/root/GameDataManager");
			System.Threading.Tasks.Task.Run(() => {
				Mesh = LoadMesh(modelPath, gameDataManager);
			});
		} else {
			delayedLoad = true;
		}
	}

	const uint MDL0_MAGIC = 0x304c444d;
	const uint MDL1_MAGIC = 0x314c444d;
	const uint MDL2_MAGIC = 0x324c444d;
	const uint GEO1_MAGIC = 0x314f4547;
	const uint COLD_MAGIC = 0x444c4f43;

	const ushort VERTEX_HAS_VECTOR = 0b0001;
	const ushort VERTEX_HAS_NORMAL = 0b0010;
	const ushort VERTEX_HAS_COLOUR = 0b0100;
	const ushort VERTEX_HAS_UV = 0b1000;

	struct Blend {
		public uint Effect;
		public ushort TextureID;
		public byte CoordinateIndex;
		public byte TilingInfo;
	}

	static Blend LoadBlend(File file) {
		return new Blend() {
			Effect = file.Get32(),
			TextureID = file.Get16(),
			CoordinateIndex = file.Get8(),
			TilingInfo = file.Get8()
		};
	}

	static Mesh LoadMesh(string modelPath, GameDataManager gameDataManager) {
		var path = gameDataManager.ResolvePath(modelPath);

		File file = new File();
		file.Open(path, File.ModeFlags.Read);

		ArrayMesh mesh = new ArrayMesh();

		Texture2D[] textures = new Texture2D[0];

		while (true) {
			var chunkType = file.Get32();
			var chunkSize = file.Get32();

			var nextChunkPosition = file.GetPosition() + chunkSize;
			switch (chunkType) {

				case MDL0_MAGIC:
					// This isn't a chunked type, so I can't skip over it
					// I'll just return
					GD.PrintErr("MDL0 Encountered: ", modelPath);
					file.Close();
					return mesh;

				case MDL2_MAGIC:
					file.Seek(file.GetPosition() + 12 + 12 + 12 + 12 + 12 + 4 + 16 + 48);

					var nTextures = file.Get32();
					textures = new Texture2D[nTextures];

					for (int i = 0; i < nTextures; i++) {
						var texturePath = file.GetFixedString(256);
						textures[i] = MIP.LoadTexture(texturePath, gameDataManager);
						file.Get32();
						file.Get32();
					}

					break;

				case GEO1_MAGIC: {
						var nDetailLevels = file.Get32();

						// for (int detailLevel = 0; detailLevel < nDetailLevels; detailLevel++) {
						// I only care about the first detailLevel
						var detailLevelType = file.Get32();
						file.GetFloat();
						var nRenderGroups = file.Get32();
						file.Get64();

						for (int renderGroup = 0; renderGroup < nRenderGroups; renderGroup++) {
							var nPolygons = file.Get16();
							var nVertices = file.Get16();
							var materialID = file.Get16();

							file.Seek(file.GetPosition() + 2 + 12 + 8);
							var blends = new Blend[] {
									LoadBlend(file),
									LoadBlend(file),
									LoadBlend(file),
									LoadBlend(file)
								};

							var groupArrays = new Godot.Collections.Array();
							groupArrays.Resize((int)ArrayMesh.ArrayType.Max);

							LoadVertices(file, ref groupArrays);

							var fillSelectablePrimBlocks = file.Get32();
							var fillType = file.Get32();
							var nIndices = file.Get32();

							var indices = new int[nIndices];

							for (int index = 0; index < nIndices; index++) {
								indices[index] = (int)file.Get16();
							}

							groupArrays[(int)ArrayMesh.ArrayType.Index] = indices;

							// if (detailLevelType == 1) {
							if (fillType == 0)
								mesh.AddSurfaceFromArrays(Mesh.PrimitiveType.Triangles, groupArrays);
							else
								mesh.AddSurfaceFromArrays(Mesh.PrimitiveType.TriangleStrip, groupArrays);

							var mat = new StandardMaterial3D();
							mat.Transparency = BaseMaterial3D.TransparencyEnum.AlphaScissor;
							mat.AlphaScissorThreshold = 0.5f;
							mat.AlbedoTexture = textures[blends[0].TextureID];
							mesh.SurfaceSetMaterial(renderGroup, mat);
							// }
						}
						// }
					}
					break;

				case 0:
					// End of File
					// Do final work here
					file.Close();
					return mesh;
			}
			file.Seek(nextChunkPosition);
		}
	}

	static void LoadVertices(File file, ref Godot.Collections.Array groupArrays) {
		var vertexOffsetVector = file.Get32();
		var vertexOffsetNormal = file.Get32();
		var vertexOffsetColour = file.Get32();
		var vertexOffsetTexcoord = file.Get32();

		var vertexSize = file.Get32();

		var nTexcoords = file.Get32();

		var flags = file.Get16();
		var nVertices = file.Get16();
		var managedBuffer = file.Get16();
		var currentVertex = file.Get16();

		var startPosition = file.GetPosition() + 8;

		var vectors = new Vector3[nVertices];
		var normals = new Vector3[nVertices];
		var colours = new Color[nVertices];
		var uv = new Vector2[nVertices];
		var uv2 = new Vector2[nVertices];

		for (var vertex = 0; vertex < nVertices; vertex++) {
			if ((flags & VERTEX_HAS_VECTOR) != 0) {
				file.Seek(startPosition + vertex * vertexSize + vertexOffsetVector);
				vectors[vertex] = file.GetVector3();
			}
			if ((flags & VERTEX_HAS_NORMAL) != 0) {
				file.Seek(startPosition + vertex * vertexSize + vertexOffsetNormal);
				normals[vertex] = file.GetVector3();
			}
			if ((flags & VERTEX_HAS_COLOUR) != 0) {
				file.Seek(startPosition + vertex * vertexSize + vertexOffsetColour);
				colours[vertex] = file.GetColorFloat();
			}
			if ((flags & VERTEX_HAS_UV) != 0) {
				file.Seek(startPosition + vertex * vertexSize + vertexOffsetTexcoord);
				uv[vertex] = file.GetVector2();
				if (nTexcoords > 1) {
					uv2[vertex] = file.GetVector2();
					//Godot only supports 2 sets of uv
					//Shouldn't matter as LR2 probably doesn't either
				}
			}
		}

		if ((flags & VERTEX_HAS_VECTOR) != 0)
			groupArrays[(int)ArrayMesh.ArrayType.Vertex] = vectors;
		if ((flags & VERTEX_HAS_NORMAL) != 0)
			groupArrays[(int)ArrayMesh.ArrayType.Normal] = normals;
		if ((flags & VERTEX_HAS_COLOUR) != 0)
			groupArrays[(int)ArrayMesh.ArrayType.Color] = colours;
		if ((flags & VERTEX_HAS_UV) != 0) {
			groupArrays[(int)ArrayMesh.ArrayType.TexUv] = uv;
			if (nTexcoords > 1)
				groupArrays[(int)ArrayMesh.ArrayType.TexUv2] = uv2;
		}

		file.Seek(startPosition + nVertices * vertexSize);
	}
}