using Godot;

public class Gizmo : Spatial, IControl {
	public static PackedScene scene = GD.Load<PackedScene>("res://Gizmo/Gizmo.tscn");

	Camera camera;

	Transform _targetTransform;
	public Transform targetTransform {
		get => _targetTransform;
		set {
			_targetTransform = value;
			EmitSignal(nameof(ValueSet), _targetTransform);
			Translation = _targetTransform.origin;
		}
	}

	public void Update(object value) {

		_targetTransform = (Transform)value;
	}

	[Signal]
	public delegate void ValueSet(Godot.Object transform);

	public override void _Ready() {
		camera = GetViewport().GetCamera();
	}

	public override void _Process(float delta) {
		Translation = _targetTransform.origin;

		var distance = camera.Translation.DistanceTo(GlobalTransform.origin);
		Scale = Vector3.One * distance;
	}
}