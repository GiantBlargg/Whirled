using Godot;
using Controls;

public partial class Gizmo : Node3D, IControl<Transform3D> {
	public static PackedScene scene = GD.Load<PackedScene>("res://Gizmo/Gizmo.tscn");

	Camera3D camera;

	Transform3D _targetTransform;
	public Transform3D targetTransform {
		get => _targetTransform;
		set {
			_targetTransform = value;
			Position = _targetTransform.origin;
			ValueSet();
		}
	}

	public Transform3D Value {
		get => _targetTransform;
		set => _targetTransform = value;
	}

	public event ValueSet ValueSet;

	public override void _Ready() {
		camera = GetViewport().GetCamera();
	}

	public override void _Process(float delta) {
		Position = _targetTransform.origin;

		var distance = camera.Position.DistanceTo(GlobalTransform.origin);
		Scale = Vector3.One * distance;
	}
}