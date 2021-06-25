using Godot;

public abstract partial class GizmoBase : StaticBody3D {
	[Export]
	public Color color;

	protected Gizmo parent;

	protected Camera3D camera;

	protected Window window;

	public override void _Ready() {
		var mesh = GetChild<MeshInstance3D>(0);
		var mat = mesh.GetSurfaceOverrideMaterial(0).Duplicate() as BaseMaterial3D;
		mat.AlbedoColor = color * mat.AlbedoColor;
		mesh.SetSurfaceOverrideMaterial(0, mat);

		parent = GetParent<Gizmo>();
		camera = GetViewport().GetCamera();

		window = GetTree().Root;

		SetProcessInput(false);
		SetProcessUnhandledInput(false);

		MouseEntered += MouseEnter;
		MouseExited += MouseExit;
	}

	public float skewCalc(Vector2 mousePos) {
		var camOrigin = camera.ProjectRayOrigin(mousePos);
		var camDirection = camera.ProjectRayNormal(mousePos);

		var globalDirection = GlobalTransform.basis.Column0.Normalized();

		var n = camDirection.Cross(camDirection.Cross(globalDirection));

		return (camOrigin - GlobalTransform.origin).Dot(n) / globalDirection.Dot(n);
	}

	public Vector2 planePosition(Vector2 mousePos) {
		var camOrigin = camera.ProjectRayOrigin(mousePos);
		var camDirection = camera.ProjectRayNormal(mousePos);

		var planeOrigin = GlobalTransform.origin;

		var globalTangent = GlobalTransform.basis.Column1.Normalized();
		var globalBitangent = GlobalTransform.basis.Column2.Normalized();

		var originOffset = camOrigin - planeOrigin;

		var M = new Basis(globalTangent, globalBitangent, camDirection).Determinant();
		var u = new Basis(originOffset, globalBitangent, camDirection).Determinant() / M;
		var v = new Basis(camDirection, globalTangent, originOffset).Determinant() / M;

		return new Vector2(u, v);
	}

	public void MouseEnter() {
		SetProcessUnhandledInput(true);
	}
	public void MouseExit() {
		SetProcessUnhandledInput(false);
	}

	public override void _UnhandledInput(InputEvent ev) {
		if (ev is InputEventMouseButton) {
			var buttonEvent = ev as InputEventMouseButton;
			if (buttonEvent.ButtonIndex == MouseButton.Left) {
				if (buttonEvent.Pressed) {
					InteractStart(buttonEvent);
					SetProcessInput(true);
					window.SetInputAsHandled();
				}
			}
		}
	}

	public override void _Input(InputEvent ev) {
		if (ev is InputEventMouseButton) {
			var buttonEvent = ev as InputEventMouseButton;
			if (buttonEvent.ButtonIndex == MouseButton.Left) {
				if (!buttonEvent.Pressed) {
					SetProcessInput(false);
					InteractEnd(buttonEvent);
				}
			}
		} else if (ev is InputEventMouseMotion) {
			var motion = ev as InputEventMouseMotion;
			var targetTransform = parent.targetTransform;
			InteractUpdate(motion, ref targetTransform);
			parent.targetTransform = targetTransform;
		}
	}

	public virtual void InteractStart(InputEventMouseButton ev) { }
	public virtual void InteractUpdate(InputEventMouseMotion ev, ref Transform3D targetTransform) { }
	public virtual void InteractEnd(InputEventMouseButton ev) { }
}