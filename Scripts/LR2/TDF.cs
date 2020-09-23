using Godot;
using System.Collections.Generic;

public class TDF : Node3D {
	private string _modelPath;
	public string ModelPath {
		get { return _modelPath; }
		set {
			_modelPath = value;
			LoadModel();
		}
	}
	private Vector2 _textureScale;
	public Vector2 textureScale {
		get { return _textureScale; }
		set {
			_textureScale = value;
			LoadModel();
		}
	}

	private bool delayedLoad = false;

	public override void _EnterTree() {
		if (delayedLoad)
			LoadModel();
	}

	void LoadModel() {
		if (IsInsideTree()) {
			var gameDataManager = GetNode<GameDataManager>("/root/Main/GameDataManager");
			System.Threading.Tasks.Task.Run(() => {
				var meshes = LoadMesh(ModelPath, textureScale, gameDataManager);
				foreach (Node child in GetChildren()) {
					child.QueueFree();
				}
				foreach (var mesh in meshes) {
					var meshInstance = new MeshInstance3D();
					meshInstance.Mesh = mesh;
					AddChild(meshInstance);
				}
			});
		} else {
			delayedLoad = true;
		}
	}

	const int CHUNK_WIDTH = 16;
	const int VERTEX_CHUNK = CHUNK_WIDTH + 1;
	const int NUM_CHUNKS = 32;

	static float MixRatioHelper(ushort src, ushort mask) {
		return ((float)(src & mask)) / (float)mask;
	}

	static void BuildSection(File file, int xPos, int yPos, float heightScale, Vector2 textureScale, ref ArrayMesh mesh) {
		var vertices = new Vector3[VERTEX_CHUNK * VERTEX_CHUNK];
		var normals = new Vector3[VERTEX_CHUNK * VERTEX_CHUNK];
		var uv = new Vector2[VERTEX_CHUNK * VERTEX_CHUNK];
		var cutout = new bool[VERTEX_CHUNK * VERTEX_CHUNK];
		var colour = new Color[VERTEX_CHUNK * VERTEX_CHUNK];//I'm using the colour info for texture mix info.

		for (int sz = 0; sz < VERTEX_CHUNK; sz++) {
			for (int sx = 0; sx < VERTEX_CHUNK; sx++) {
				var index = sz * VERTEX_CHUNK + sx;
				vertices[index] = new Vector3(
						xPos + sx - CHUNK_WIDTH * NUM_CHUNKS / 2,
						file.Get16() * heightScale,
						yPos + sz - CHUNK_WIDTH * NUM_CHUNKS / 2);
				normals[index] = new Vector3((sbyte)file.Get8(), (sbyte)file.Get8(), (sbyte)file.Get8()).Normalized();
				uv[index] = new Vector2(
						(float)(xPos + sx) / CHUNK_WIDTH,
						(float)(yPos + sz) / CHUNK_WIDTH)
						* textureScale;

				cutout[index] = (file.Get8() & 0b10000000) == 0b10000000;

				var mixRatios = file.Get16();
				colour[index] = new Color(
						MixRatioHelper(mixRatios, 0x000f),
						MixRatioHelper(mixRatios, 0x00f0),
						MixRatioHelper(mixRatios, 0x0f00),
						MixRatioHelper(mixRatios, 0xf000)
						);
			}
		}

		var indices = new int[CHUNK_WIDTH * CHUNK_WIDTH * 6];

		for (var z = 0; z < CHUNK_WIDTH; z++) {
			for (var x = 0; x < CHUNK_WIDTH; x++) {
				var index = (z * CHUNK_WIDTH + x) * 6;
				var base_vertex = (z * VERTEX_CHUNK + x);

				if (!cutout[base_vertex + 1 + VERTEX_CHUNK]) {
					if (z % 2 == 0) {
						indices[index] = base_vertex;
						indices[index + 1] = base_vertex + 1;
						indices[index + 2] = base_vertex + VERTEX_CHUNK;
						indices[index + 3] = base_vertex + VERTEX_CHUNK;
						indices[index + 4] = base_vertex + 1;
						indices[index + 5] = base_vertex + 1 + VERTEX_CHUNK;
					} else {
						indices[index] = base_vertex;
						indices[index + 1] = base_vertex + 1;
						indices[index + 2] = base_vertex + 1 + VERTEX_CHUNK;
						indices[index + 3] = base_vertex;
						indices[index + 4] = base_vertex + 1 + VERTEX_CHUNK;
						indices[index + 5] = base_vertex + VERTEX_CHUNK;
					}
				}
			}
		}

		var array = new Godot.Collections.Array();
		array.Resize((int)ArrayMesh.ArrayType.Max);
		array[(int)ArrayMesh.ArrayType.Vertex] = vertices;
		array[(int)ArrayMesh.ArrayType.Normal] = normals;
		array[(int)ArrayMesh.ArrayType.Color] = colour;
		array[(int)ArrayMesh.ArrayType.TexUv] = uv;
		array[(int)ArrayMesh.ArrayType.Index] = indices;
		mesh.AddSurfaceFromArrays(Mesh.PrimitiveType.Triangles, array);
	}

	static Mesh[] LoadMesh(string modelPath, Vector2 textureScale, GameDataManager gameDataManager) {

		TDFMaterialFactory materialFactory = new TDFMaterialFactory(modelPath, gameDataManager);

		var terrDataPath = gameDataManager.ResolvePath(modelPath + "/TERRDATA.TDF");
		var file = new File();
		file.Open(terrDataPath, File.ModeFlags.Read);

		file.Seek(0x10);

		var heightScale = file.GetFloat();

		var meshes = new ArrayMesh[NUM_CHUNKS * NUM_CHUNKS];

		for (int i = 0; i < NUM_CHUNKS * NUM_CHUNKS; i++) {
			if (i == 0x3A2) {
				GD.Print();
			}
			file.Seek(0x3AE020 + i * 4);
			var surfaceOffset = file.Get32();
			file.Seek(0x366020 + surfaceOffset + 3 * 4);
			ushort xPos = file.Get16();
			ushort yPos = file.Get16();

			file.Seek(0x366020 + surfaceOffset + 0x90);
			uint verticiesOffset = file.Get32();

			file.Seek(0x20 + verticiesOffset);
			var mesh = new ArrayMesh();
			BuildSection(file, xPos, yPos, heightScale, textureScale, ref mesh);

			file.Seek(0x366020 + surfaceOffset + 0x118);
			byte[] textureIDs = file.GetBuffer(4);

			var mat = materialFactory.GetMaterial(textureIDs);
			mesh.SurfaceSetMaterial(0, mat);

			meshes[i] = mesh;
		}

		return meshes;
	}
}

class TDFMaterialFactory {

	static Shader shader = GD.Load<Shader>("res://Scripts/LR2/TDF.shader");

	List<Texture> textures = new List<Texture>();
	Dictionary<byte[], ShaderMaterial> materials = new Dictionary<byte[], ShaderMaterial>();

	public TDFMaterialFactory(string modelPath, GameDataManager gameDataManager) {
		int i = 0;
		while (true) {
			var tex = MIP.LoadTexture(modelPath + "/TEXTURE" + (i + 1) + ".TGA", gameDataManager);
			if (tex == null) break;
			textures.Add(tex);
			i++;
		}
	}
	public ShaderMaterial GetMaterial(byte[] textureIDs) {
		if (!materials.ContainsKey(textureIDs)) {
			var mat = new ShaderMaterial();
			mat.Shader = shader;

			for (int i = 0; i < 4; i++) {
				if (textureIDs[i] == 0xff) break;

				mat.SetShaderParam($"tex{i}", textures[textureIDs[i]]);
			}

			materials[textureIDs] = mat;
		}
		return materials[textureIDs];
	}
}