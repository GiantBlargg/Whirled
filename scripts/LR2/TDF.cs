using Godot;

public class TDF : MeshInstance3D {
	public string modelPath {
		get { return _modelPath; }
		set {
			_modelPath = value;
			LoadModel();
		}
	}

	public Vector2 textureScale { get; set; }

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

	const int CHUNK_WIDTH = 16;
	const int VERTEX_CHUNK = CHUNK_WIDTH + 1;
	const int NUM_CHUNKS = 32;
	const int VERTEX_OFFSET = 0x20;

	static Mesh LoadMesh(string modelPath, GameDataManager gameDataManager) {
		var terrDataPath = gameDataManager.ResolvePath(modelPath + "/TERRDATA.TDF");
		var file = new File();
		file.Open(terrDataPath, File.ModeFlags.Read);

		file.Seek(VERTEX_OFFSET);

		var indices = new int[CHUNK_WIDTH * CHUNK_WIDTH * 6];

		for (var z = 0; z < CHUNK_WIDTH; z++) {
			for (var x = 0; x < CHUNK_WIDTH; x++) {
				var index = (z * CHUNK_WIDTH + x) * 6;
				var base_vertex = (z * VERTEX_CHUNK + x);
				indices[index] = base_vertex;
				indices[index + 1] = base_vertex + 1;
				indices[index + 2] = base_vertex + VERTEX_CHUNK;
				indices[index + 3] = base_vertex + VERTEX_CHUNK;
				indices[index + 4] = base_vertex + 1;
				indices[index + 5] = base_vertex + 1 + VERTEX_CHUNK;
			}
		}

		var mesh = new ArrayMesh();

		for (int cx = 0; cx < NUM_CHUNKS; cx++) {
			for (int cz = 0; cz < NUM_CHUNKS; cz++) {

				var vertices = new Vector3[VERTEX_CHUNK * VERTEX_CHUNK];
				var normals = new Vector3[VERTEX_CHUNK * VERTEX_CHUNK];

				for (int sz = 0; sz < VERTEX_CHUNK; sz++) {
					for (int sx = 0; sx < VERTEX_CHUNK; sx++) {
						var index = sz * VERTEX_CHUNK + sx;
						vertices[index] = new Vector3(
								cx * CHUNK_WIDTH + sx - CHUNK_WIDTH * NUM_CHUNKS / 2,
								(file.Get16() - 56) / 64f, //Determined by trial and error; TODO: Find a more accurate conversion
								cz * CHUNK_WIDTH + sz - CHUNK_WIDTH * NUM_CHUNKS / 2);
						normals[index] = new Vector3((sbyte)file.Get8(), (sbyte)file.Get8(), (sbyte)file.Get8());

						file.Seek(file.GetPosition() + 3);
					}
				}
				var array = new Godot.Collections.Array();
				array.Resize((int)ArrayMesh.ArrayType.Max);
				array[(int)ArrayMesh.ArrayType.Vertex] = vertices;
				array[(int)ArrayMesh.ArrayType.Normal] = normals;
				array[(int)ArrayMesh.ArrayType.Index] = indices;
				mesh.AddSurfaceFromArrays(Mesh.PrimitiveType.Triangles, array);
			}
		}

		return mesh;
	}
}