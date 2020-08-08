using Godot;

namespace Controls {
	public class TransformControl : VBoxContainer, IControl<Transform> {
		static float RadToDeg = 180 / Mathf.Pi;
		static float DegToRad = Mathf.Pi / 180;

		Vector3Control translate, rotation, scale;

		bool AllowScale;

		public TransformControl(bool allowScale = false) {
			AllowScale = allowScale;

			SizeFlagsHorizontal = (int)Control.SizeFlags.ExpandFill;

			translate = AddComponent("Translation");
			rotation = AddComponent("Rotation");

			if (AllowScale) {
				scale = AddComponent("Scale");
			}
		}

		Vector3Control AddComponent(string name) {
			var component = new Vector3Control();
			component.Name = name;
			component.ValueSet += h => ValueSet(h);
			AddChild(component);
			return component;
		}

		public event ValueSet ValueSet;

		public void SetTransform(string _) => ValueSet();
		public void SetTransform() => ValueSet();

		public Transform Value {
			get {
				var basis = new Basis(rotation.Value * DegToRad);
				if (AllowScale) {
					basis = basis.Scaled(scale.Value);
				}
				return new Transform(basis, translate.Value);
			}
			set {
				translate.Value = value.origin;
				rotation.Value = value.basis.GetEuler() * RadToDeg;
				if (AllowScale) {
					scale.Value = value.basis.Scale;
				}
			}
		}
	}
}