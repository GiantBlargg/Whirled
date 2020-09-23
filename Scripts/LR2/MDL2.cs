using Godot;
using System.Collections.Generic;

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

	//Not the COLD Section
	//Generated from mesh
	public CollisionObject3D collider = new Area3D();
	public CollisionShape3D shape = new CollisionShape3D();

	public override void _Ready() {
		collider.AddChild(shape);
		AddChild(collider);
	}


	void LoadModel() {
		if (IsInsideTree()) {
			var gameDataManager = GetNode<GameDataManager>("/root/Main/GameDataManager");
			// System.Threading.Tasks.Task.Run(() => {
			(Mesh, shape.Shape) = LoadMesh(modelPath, gameDataManager);
			// });
		} else {
			delayedLoad = true;
		}
	}

	const uint MDL0_MAGIC = 0x304c444d;
	const uint MDL1_MAGIC = 0x314c444d;
	const uint MDL2_MAGIC = 0x324c444d;
	const uint GEO0_MAGIC = 0x304f4547;
	const uint GEO1_MAGIC = 0x314f4547;
	const uint P2G0_MAGIC = 0x30473250;
	const uint COLD_MAGIC = 0x444c4f43;
	const uint SHA0_MAGIC = 0x30414853;

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

	struct MaterialProps {
		public Color ambient;
		public Color diffuse;
		public Color specular;
		public Color emissive;
		public float shine;
		public float alpha;
		public uint alphaType;
		public uint bitfield;
		public string animName;
	}

	static (Mesh, Shape3D) LoadMesh(string modelPath, GameDataManager gameDataManager) {
		var path = gameDataManager.ResolvePath(modelPath);

		File file = new File();
		file.Open(path, File.ModeFlags.Read);

		ArrayMesh mesh = new ArrayMesh();
		var shape = new List<Vector3>();

		Texture2D[] textures = new Texture2D[0];
		MaterialProps[] materialProps = new MaterialProps[0];

		while (true) {
			var chunkType = file.Get32();
			var chunkSize = file.Get32();

			var nextChunkPosition = file.GetPosition() + chunkSize;
			switch (chunkType) {

				default:
					GD.Print("Unkown Chunk! ", modelPath);
					break;

				case MDL0_MAGIC:
					// This isn't a chunked type, so I can't skip over it
					// I'll just return
					GD.PrintErr("MDL0: ", modelPath);
					file.Close();
					return (mesh, new BoxShape3D());

				case MDL1_MAGIC:
					// GD.PrintErr("MDL1: ", modelPath);
					break;

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

					var nMaterials = file.Get32();
					materialProps = new MaterialProps[nMaterials];

					for (int i = 0; i < nMaterials; i++) {
						materialProps[i] = new MaterialProps() {
							ambient = file.GetColorRGBAf(),
							diffuse = file.GetColorRGBAf(),
							specular = file.GetColorRGBAf(),
							emissive = file.GetColorRGBAf(),
							shine = file.GetFloat(),
							alpha = file.GetFloat(),
							alphaType = file.Get32(),
							bitfield = file.Get32(),
							animName = file.GetFixedString(8)
						};
					}

					break;

				case GEO0_MAGIC:
					// GD.PrintErr("GEO0: ", modelPath);
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

							var verts = new List<Vector3>();
							LoadVertices(file, ref groupArrays, ref verts);

							var fillSelectablePrimBlocks = file.Get32();
							var fillType = file.Get32();
							var nIndices = file.Get32();

							var indices = new int[nIndices];

							for (int index = 0; index < nIndices; index++) {
								indices[index] = (int)file.Get16();
								shape.Add(verts[indices[index]]);
							}

							groupArrays[(int)ArrayMesh.ArrayType.Index] = indices;

							// if (detailLevelType == 1) {
							if (fillType == 0)
								mesh.AddSurfaceFromArrays(Mesh.PrimitiveType.Triangles, groupArrays);
							else
								mesh.AddSurfaceFromArrays(Mesh.PrimitiveType.TriangleStrip, groupArrays);

							if (fillType != 0)
								GD.PrintErr("Triangle Strip surface! Collision will be incorrect!");

							var materialProp = materialProps[materialID];

							var mat = new StandardMaterial3D();

							mat.CullMode = BaseMaterial3D.CullModeEnum.Front;

							mat.DiffuseMode = BaseMaterial3D.DiffuseModeEnum.Lambert;
							mat.SpecularMode = BaseMaterial3D.SpecularModeEnum.Disabled;

							float alpha = 1 - materialProp.alpha;

							switch (materialProp.alphaType) {
								case 0:
									mat.Transparency = BaseMaterial3D.TransparencyEnum.AlphaScissor;
									mat.AlphaScissorThreshold = 0.5f;
									break;
								case 1:
									mat.BlendMode = BaseMaterial3D.BlendModeEnum.Mix;
									mat.Transparency = BaseMaterial3D.TransparencyEnum.Alpha;
									break;
								case 4:
									mat.BlendMode = BaseMaterial3D.BlendModeEnum.Add;
									mat.CullMode = BaseMaterial3D.CullModeEnum.Disabled;
									mat.ShadingMode = BaseMaterial3D.ShadingModeEnum.Unshaded;
									break;
								default:
									GD.PrintErr("Unkown Alpha Type ", materialProp.alphaType, " in ", path);
									break;
							}


							mat.AlbedoColor = new Color(1, 1, 1, alpha);
							// mat.AlbedoColor = materialProp.diffuse;
							// mat.Emission = materialProp.emissive;
							// mat.Roughness = materialProp.shine;

							mat.AlbedoTexture = textures[blends[0].TextureID];
							mesh.SurfaceSetMaterial(renderGroup, mat);
							// }
						}
						// }
					}
					break;

				case P2G0_MAGIC:
					//I don't know what this is
					// GD.PrintErr("P2G0: ", modelPath);
					break;

				case SHA0_MAGIC:
					//I don't know what this is
					// GD.PrintErr("SHA0: ", modelPath);
					break;

				case COLD_MAGIC:
					//Collision
					// GD.PrintErr("COLD: ", modelPath);
					break;

				case 0:
					// End of File
					// Do final work here
					file.Close();
					var c = new ConcavePolygonShape3D();
					c.Data = shape.ToArray();
					return (mesh, c);
			}
			file.Seek(nextChunkPosition);
		}
	}

	static void LoadVertices(File file, ref Godot.Collections.Array groupArrays, ref List<Vector3> shape) {
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
				colours[vertex] = file.GetColorRGBAf();
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

		if ((flags & VERTEX_HAS_VECTOR) != 0) {
			groupArrays[(int)ArrayMesh.ArrayType.Vertex] = vectors;
			shape.AddRange(vectors);
		}
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