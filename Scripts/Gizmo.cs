using Godot;

public class GizmoTarget : Spatial {
	public bool AllowScale = false;

	[Signal]
	public delegate void TransformChanged(Transform transform);

	public override void _Ready() {
		SetNotifyTransform(true);
		EmitSignal(nameof(TransformChanged), Transform);
	}

	public override void _Notification(int what) {
		if (what == NotificationTransformChanged)
			EmitSignal(nameof(TransformChanged), Transform);
	}
}

public class TransformUI : VBoxContainer {
	static Vector3 HandednessTransform = new Vector3(-1, 1, 1);
	static Vector3 RotateHandednessTransform = new Vector3(1, -1, -1);
	static float RadToDeg = 180 / Mathf.Pi;
	static float DegToRad = Mathf.Pi / 180;

	TransformUISubSection translate, rotation, scale;

	GizmoTarget target;
	public TransformUI(GizmoTarget _target) {
		target = _target;
		target.Connect(nameof(GizmoTarget.TransformChanged), this, nameof(Update));

		SizeFlagsHorizontal = (int)Control.SizeFlags.ExpandFill;

		var transLabel = new Label();
		transLabel.Text = "Translation";
		AddChild(transLabel);
		translate = new TransformUISubSection(this);

		var rotLabel = new Label();
		rotLabel.Text = "Rotation";
		AddChild(rotLabel);
		rotation = new TransformUISubSection(this);

		if (target.AllowScale) {
			var scaleLable = new Label();
			scaleLable.Text = "Scale";
			AddChild(scaleLable);
			scale = new TransformUISubSection(this);
		}
	}

	public void SetTransform(string _ = "") {
		var basis = new Basis(rotation.value * RotateHandednessTransform * DegToRad);
		if (target.AllowScale) {
			basis = basis.Scaled(scale.value * HandednessTransform);
		}
		var transform = new Transform(basis, translate.value * HandednessTransform);
		target.Transform = transform;
	}

	public void Update(Transform transform) {
		translate.Update(transform.origin * HandednessTransform);
		rotation.Update(transform.basis.GetEuler() * RotateHandednessTransform * RadToDeg);
		if (target.AllowScale) {
			scale.Update(transform.basis.Scale * HandednessTransform);
		}
	}
}

class TransformUISubSection : HBoxContainer {
	LineEdit x, y, z;
	public TransformUISubSection(TransformUI parent) {
		x = CreateComponent(parent);
		y = CreateComponent(parent);
		z = CreateComponent(parent);

		parent.AddChild(this);
	}

	LineEdit CreateComponent(TransformUI parent) {
		var component = new EnterReleaseFocusLineEdit();

		component.SizeFlagsHorizontal = (int)Control.SizeFlags.ExpandFill;

		component.Connect("text_entered", parent, nameof(TransformUI.SetTransform));

		AddChild(component);
		return component;
	}

	float Parse(string v) {
		float f;
		if (float.TryParse(v, out f)) return f;
		return 0;
	}

	public Vector3 value {
		get {
			return new Vector3(
				Parse(x.Text),
				Parse(y.Text),
				Parse(z.Text)
			);
		}
		set { Update(value); }
	}

	public void Update(Vector3 vector) {
		UpdateComponent(x, vector.x);
		UpdateComponent(y, vector.y);
		UpdateComponent(z, vector.z);
	}
	static void UpdateComponent(LineEdit line, float value) {
		line.Text = value.ToString();
	}
}

class EnterReleaseFocusLineEdit : LineEdit {
	public override void _Ready() {
		Connect("text_entered", this, nameof(LoseFocus));
	}
	void LoseFocus(string _ = "") {
		ReleaseFocus();
	}
}