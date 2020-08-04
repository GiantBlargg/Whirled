using Godot;

public abstract class GizmoBase : StaticBody {
	[Export]
	public Color color;

	protected Gizmo parent;

	protected Camera camera;

	protected SceneTree tree;

	public override void _Ready() {
		var mesh = GetChild<MeshInstance>(0);
		var mat = mesh.GetSurfaceMaterial(0).Duplicate() as SpatialMaterial;
		mat.AlbedoColor = color * mat.AlbedoColor;
		mesh.SetSurfaceMaterial(0, mat);

		parent = GetParent<Gizmo>();
		camera = GetViewport().GetCamera();

		tree = GetTree();

		SetProcessInput(false);
		SetProcessUnhandledInput(false);

		Connect("mouse_entered", this, nameof(MouseEnter));
		Connect("mouse_exited", this, nameof(MouseExit));
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
			if (buttonEvent.ButtonIndex == (int)ButtonList.Left) {
				if (buttonEvent.Pressed) {
					InteractStart(buttonEvent);
					SetProcessInput(true);
					tree.SetInputAsHandled();
				}
			}
		}
	}

	public override void _Input(InputEvent ev) {
		if (ev is InputEventMouseButton) {
			var buttonEvent = ev as InputEventMouseButton;
			if (buttonEvent.ButtonIndex == (int)ButtonList.Left) {
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
	public virtual void InteractUpdate(InputEventMouseMotion ev, ref Transform targetTransform) { }
	public virtual void InteractEnd(InputEventMouseButton ev) { }
}