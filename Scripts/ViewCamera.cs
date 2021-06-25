using Godot;

public partial class ViewCamera : Camera3D {
	const float lookSpeed = 0.2f;
	float moveSpeed = 100f;
	bool Right, Middle;
	Window window;

	public override void _EnterTree() {
		window = GetTree().Root;
	}

	public override void _Input(InputEvent inputEvent) {
		if (Right || Middle) {
			if (inputEvent is InputEventKey) {
				window.SetInputAsHandled();
			}
		}
	}
	public override void _UnhandledInput(InputEvent inputEvent) {
		if (inputEvent is InputEventMouseButton) {
			var mouseButton = (InputEventMouseButton)inputEvent;
			if (mouseButton.ButtonIndex == MouseButton.Right) {
				Right = mouseButton.Pressed;
				Input.SetMouseMode(mouseButton.Pressed ? Input.MouseMode.Captured : Input.MouseMode.Visible);
			}
		} else if (inputEvent is InputEventMouseMotion) {
			var mouseMotion = (InputEventMouseMotion)inputEvent;
			if (Input.IsMouseButtonPressed(MouseButton.Right)) {
				var newRot = RotationDegrees + new Vector3(mouseMotion.Relative.y, mouseMotion.Relative.x, 0) * -lookSpeed;
				if (newRot.x < -90) newRot.x = -90;
				if (newRot.x > 90) newRot.x = 90;
				RotationDegrees = newRot;
			}
		}
	}
	public override void _Process(float delta) {
		if (Input.IsMouseButtonPressed(MouseButton.Right)) {
			var movement = new Vector3();
			if (Input.IsKeyPressed((int)Key.W))
				movement.z--;
			if (Input.IsKeyPressed((int)Key.S))
				movement.z++;
			if (Input.IsKeyPressed((int)Key.A))
				movement.x--;
			if (Input.IsKeyPressed((int)Key.D))
				movement.x++;
			if (Input.IsKeyPressed((int)Key.Q))
				movement.y--;
			if (Input.IsKeyPressed((int)Key.E))
				movement.y++;

			movement *= delta * moveSpeed;

			Translate(movement);
		}
	}
}
