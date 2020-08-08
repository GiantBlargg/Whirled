using Godot;

namespace Controls {
	public class TransformControl : VBoxContainer, IControl<Transform> {
		static float RadToDeg = 180 / Mathf.Pi;
		static float DegToRad = Mathf.Pi / 180;

		TransformControlSubSection translate, rotation, scale;

		bool AllowScale;

		public TransformControl(bool _AllowScale = false) {
			AllowScale = _AllowScale;

			SizeFlagsHorizontal = (int)Control.SizeFlags.ExpandFill;

			var transLabel = new Label();
			transLabel.Text = "Translation";
			AddChild(transLabel);
			translate = new TransformControlSubSection(this);

			var rotLabel = new Label();
			rotLabel.Text = "Rotation";
			AddChild(rotLabel);
			rotation = new TransformControlSubSection(this);

			if (AllowScale) {
				var scaleLable = new Label();
				scaleLable.Text = "Scale";
				AddChild(scaleLable);
				scale = new TransformControlSubSection(this);
			}
		}

		public event ValueSet ValueSet;

		public void SetTransform(string _) => ValueSet();
		public void SetTransform() => ValueSet();

		public Transform Value {
			get {
				var basis = new Basis(rotation.value * DegToRad);
				if (AllowScale) {
					basis = basis.Scaled(scale.value);
				}
				return new Transform(basis, translate.value);
			}
			set {
				translate.Update(value.origin);
				rotation.Update(value.basis.GetEuler() * RadToDeg);
				if (AllowScale) {
					scale.Update(value.basis.Scale);
				}
			}
		}
	}

	class TransformControlSubSection : HBoxContainer {
		LineEdit x, y, z;
		public TransformControlSubSection(TransformControl parent) {
			x = CreateComponent(parent);
			y = CreateComponent(parent);
			z = CreateComponent(parent);

			parent.AddChild(this);
		}

		LineEdit CreateComponent(TransformControl parent) {
			var component = new LineEdit();

			component.SizeFlagsHorizontal = (int)Control.SizeFlags.ExpandFill;

			component.Connect("text_entered", parent, nameof(TransformControl.SetTransform));
			component.Connect("focus_exited", parent, nameof(TransformControl.SetTransform));

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
}