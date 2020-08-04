using Godot;

public class Plane : GizmoBase {
	Vector2 offset;

	public override void InteractStart(InputEventMouseButton ev) => offset = planePosition(ev.Position);
	public override void InteractUpdate(InputEventMouseMotion ev, ref Transform targetTransform) {
		var p = planePosition(ev.Position) - offset;
		targetTransform.origin += p.x * Transform.basis.Column1.Normalized() + p.y * Transform.basis.Column2.Normalized();
	}
}
