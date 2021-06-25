using Godot;
using System.Collections.Generic;

public partial class MDL2 : MeshInstance3D {
	public string modelPath {
		get { return _modelPath; }
		set {
			_modelPath = value;
			LoadModel();
		}
	}
	private string _modelPath;

	//Not the COLD Section
	//Generated from mesh
	public CollisionObject3D collider = new Area3D();
	public CollisionShape3D shape = new CollisionShape3D();

	public override void _Ready() {
		collider.AddChild(shape);
		AddChild(collider);
	}

	void LoadModel() {
		ResourceLoader.LoadThreadedRequest("lr2://" + modelPath, useSubThreads: true);
		SetProcess(true);
	}

	public override void _Process(float delta) {
		if (modelPath == null || modelPath == "" || (Mesh != null && "lr2://" + modelPath == Mesh.ResourcePath)) {
			SetProcess(false);
			return;
		}
		var status = ResourceLoader.LoadThreadedGetStatus("lr2://" + modelPath);
		switch (status) {
			case ResourceLoader.ThreadLoadStatus.InvalidResource:
			case ResourceLoader.ThreadLoadStatus.Failed:
				SetProcess(false);
				return;
			case ResourceLoader.ThreadLoadStatus.InProgress:
				return;
			case ResourceLoader.ThreadLoadStatus.Loaded:
				Mesh = ResourceLoader.LoadThreadedGet("lr2://" + modelPath) as Mesh;
				SetProcess(false);
				return;
		}

		GD.PrintErr("This shouldn't be happening");

	}
}