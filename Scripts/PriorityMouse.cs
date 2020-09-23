using Godot;
using System;

public class PriorityMouse : Node {
	Camera3D camera;
	PhysicsDirectSpaceState3D space;
	CollisionObject3D lastCollider = null;

	public override void _Ready() {
		var viewport = GetViewport();
		camera = viewport.GetCamera();
		space = viewport.World3d.DirectSpaceState;
	}
	public override void _UnhandledInput(InputEvent e) {
		if (e is InputEventMouseMotion) {
			var mouse = e as InputEventMouseMotion;
			Vector3 from = camera.ProjectRayOrigin(mouse.Position);
			Vector3 dir = camera.ProjectRayNormal(mouse.Position);

			var result = space.IntersectRay(from, from + dir * camera.Far, collisionMask: 0x80000);

			CollisionObject3D collider = null;
			if (result.Contains("collider")) {
				collider = (CollisionObject3D)result["collider"];
			}

			if (collider != lastCollider) {
				try {
					if (lastCollider != null)
						lastCollider.EmitSignal("mouse_exited");
				} catch (ObjectDisposedException) { }
				if (collider != null)
					collider.EmitSignal("mouse_entered");

				lastCollider = collider;
			}
		}
	}
}