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
		var basis = new Basis(rotation.value);
		if (target.AllowScale) {
			basis = basis.Scaled(scale.value);
		}
		var transform = new Transform(basis, translate.value);
		target.Transform = transform;
	}

	public void Update(Transform transform) {
		translate.Update(transform.origin);
		rotation.Update(transform.basis.GetEuler());
		if (target.AllowScale) {
			scale.Update(transform.basis.Scale);
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
		var component = new LineEdit();

		component.SizeFlagsHorizontal = (int)Control.SizeFlags.ExpandFill;

		component.Connect("text_changed", parent, nameof(TransformUI.SetTransform));

		AddChild(component);
		return component;
	}

	public Vector3 value {
		get {
			return new Vector3(
				float.Parse(x.Text),
				float.Parse(y.Text),
				float.Parse(z.Text)
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
		if (!line.HasFocus())
			line.Text = value.ToString();
	}
}