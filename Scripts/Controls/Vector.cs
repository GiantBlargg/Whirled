using Godot;

namespace Controls {
	public abstract class VectorBase : VBoxContainer {
		HBoxContainer hBox = new HBoxContainer();
		public override void _Ready() {
			if (Name != "" && ((string)Name)[0] != '@') {
				var label = new Label();
				label.Text = Name;
				AddChild(label);
			}
			AddChild(hBox);
		}
		protected Number<float> CreateComponent() {
			var component = new Number<float>();

			component.SizeFlagsHorizontal = (int)Control.SizeFlags.ExpandFill;

			component.ValueSet += _ => ValueSet();

			hBox.AddChild(component);
			return component;
		}

		public event ValueSet ValueSet;
	}
	public class Vector2Control : VectorBase, IControl<Vector2> {
		Number<float> x, y;
		public Vector2Control() {
			x = CreateComponent();
			y = CreateComponent();
		}
		public Vector2 Value {
			get => new Vector2(x.Value, y.Value);
			set {
				x.Value = value.x;
				y.Value = value.y;
			}
		}
	}
	public class Vector3Control : VectorBase, IControl<Vector3> {
		Number<float> x, y, z;
		public Vector3Control() {
			x = CreateComponent();
			y = CreateComponent();
			z = CreateComponent();
		}
		public Vector3 Value {
			get => new Vector3(x.Value, y.Value, z.Value);
			set {
				x.Value = value.x;
				y.Value = value.y;
				z.Value = value.z;
			}
		}
	}
}