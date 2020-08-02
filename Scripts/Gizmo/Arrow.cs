using Godot;

public class Arrow : StaticBody {
	[Export]
	public Vector3 direction;
	public Vector3 globalDirection;

	Gizmo parent;

	Camera camera;

	float offset;

	SceneTree tree;

	public override void _Ready() {
		var up = Vector3.Up;
		if (up.Normalized() == direction.Normalized()) {
			up = Vector3.Right;
		}
		Transform = Transform.LookingAt(direction, up).Scaled(Scale);

		var mat = GetNode<MeshInstance>("arrow").GetSurfaceMaterial(0).Duplicate() as SpatialMaterial;
		mat.AlbedoColor = new Color(direction.x, direction.y, direction.z, 1) * mat.AlbedoColor;
		GetNode<MeshInstance>("arrow").SetSurfaceMaterial(0, mat);

		parent = GetParent<Gizmo>();
		camera = GetViewport().GetCamera();

		tree = GetTree();

		globalDirection = direction;

		var grandParent = parent.GetParent();
		if (grandParent is Spatial) {
			globalDirection = (grandParent as Spatial).GlobalTransform.basis.Xform(direction);
		}

		SetProcessInput(false);
		SetProcessUnhandledInput(false);

		Connect("mouse_entered", this, nameof(MouseEnter));
		Connect("mouse_exited", this, nameof(MouseExit));
	}

	public override void _Input(InputEvent ev) {
		if (ev is InputEventMouseButton) {
			var buttonEvent = ev as InputEventMouseButton;
			if (buttonEvent.ButtonIndex == (int)ButtonList.Left) {
				if (!buttonEvent.Pressed) {
					SetProcessInput(false);
				}
			}
		} else if (ev is InputEventMouseMotion) {
			var motion = ev as InputEventMouseMotion;
			var t = skewCalc(motion.Position);
			var targetTransform = parent.targetTransform;
			targetTransform.origin += (t - offset) * direction;
			parent.targetTransform = targetTransform;
		}
	}

	public float skewCalc(Vector2 mousePos) {
		var camOrigin = camera.ProjectRayOrigin(mousePos);
		var camDirection = camera.ProjectRayNormal(mousePos);

		var n = camDirection.Cross(globalDirection);

		var n1 = camDirection.Cross(n);

		var t2 = (camOrigin - GlobalTransform.origin).Dot(n1) / globalDirection.Dot(n1);

		return t2;
	}

	public override void _UnhandledInput(InputEvent ev) {
		if (ev is InputEventMouseButton) {
			var buttonEvent = ev as InputEventMouseButton;
			if (buttonEvent.ButtonIndex == (int)ButtonList.Left) {
				if (buttonEvent.Pressed) {
					SetProcessInput(true);
					offset = skewCalc(buttonEvent.Position);

					tree.SetInputAsHandled();
				}
			}
		}
	}

	public void MouseEnter() {
		SetProcessUnhandledInput(true);
		GD.Print("enter ", Name);
	}
	public void MouseExit() {
		SetProcessUnhandledInput(false);
		GD.Print("exit ", Name);
	}
}