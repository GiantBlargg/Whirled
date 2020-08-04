using Godot;

public class Ring : GizmoBase {
	float lastAngle;

	public override void InteractStart(InputEventMouseButton ev) => lastAngle = planePosition(ev.Position).Angle();
	public override void InteractUpdate(InputEventMouseMotion ev, ref Transform targetTransform) {
		var currentAngle = planePosition(ev.Position).Angle();
		targetTransform.basis = targetTransform.basis.Rotated(Transform.basis.Column0.Normalized(), currentAngle - lastAngle);
		lastAngle = currentAngle;
	}
}
