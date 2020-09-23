using Godot;
using Controls;

public class Gizmo : Node3D, IControl<Transform> {
	public static PackedScene scene = GD.Load<PackedScene>("res://Gizmo/Gizmo.tscn");

	Camera3D camera;

	Transform _targetTransform;
	public Transform targetTransform {
		get => _targetTransform;
		set {
			_targetTransform = value;
			Translation = _targetTransform.origin;
			ValueSet();
		}
	}

	public Transform Value {
		get => _targetTransform;
		set => _targetTransform = value;
	}

	public event ValueSet ValueSet;

	public override void _Ready() {
		camera = GetViewport().GetCamera();
	}

	public override void _Process(float delta) {
		Translation = _targetTransform.origin;

		var distance = camera.Translation.DistanceTo(GlobalTransform.origin);
		Scale = Vector3.One * distance;
	}
}