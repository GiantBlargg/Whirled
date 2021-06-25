using Godot;

public partial class Arrow : GizmoBase {
	float offset;

	public override void InteractStart(InputEventMouseButton ev) => offset = skewCalc(ev.Position);
	public override void InteractUpdate(InputEventMouseMotion ev, ref Transform3D targetTransform) =>
		targetTransform.origin += (skewCalc(ev.Position) - offset) * Transform.basis.Column0.Normalized();
}
