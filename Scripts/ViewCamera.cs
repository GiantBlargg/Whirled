using Godot;

public class ViewCamera : Camera {
	const float lookSpeed = 0.2f;
	float moveSpeed = 100f;
	public override void _UnhandledInput(InputEvent inputEvent) {
		if (inputEvent is InputEventMouseButton) {
			var mouseButton = (InputEventMouseButton)inputEvent;
			if (mouseButton.ButtonIndex == (int)ButtonList.Right) {
				Input.SetMouseMode(mouseButton.Pressed ? Input.MouseMode.Captured : Input.MouseMode.Visible);
			}
		} else if (inputEvent is InputEventMouseMotion) {
			var mouseMotion = (InputEventMouseMotion)inputEvent;
			if (Input.IsMouseButtonPressed((int)ButtonList.Right)) {
				var newRot = RotationDegrees + new Vector3(mouseMotion.Relative.y, mouseMotion.Relative.x, 0) * -lookSpeed;
				if (newRot.x < -90) newRot.x = -90;
				if (newRot.x > 90) newRot.x = 90;
				RotationDegrees = newRot;
			}
		}
	}
	public override void _Process(float delta) {
		if (Input.IsMouseButtonPressed((int)ButtonList.Right)) {
			var movement = new Vector3();
			if (Input.IsKeyPressed((int)KeyList.W))
				movement.z--;
			if (Input.IsKeyPressed((int)KeyList.S))
				movement.z++;
			if (Input.IsKeyPressed((int)KeyList.A))
				movement.x--;
			if (Input.IsKeyPressed((int)KeyList.D))
				movement.x++;
			if (Input.IsKeyPressed((int)KeyList.Q))
				movement.y--;
			if (Input.IsKeyPressed((int)KeyList.E))
				movement.y++;

			movement *= delta * moveSpeed;

			Translate(movement);
		}
	}
}
