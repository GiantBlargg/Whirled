using Godot;

namespace Controls {
	public abstract class StringBase : HBoxContainer {
		protected LineEdit line = new LineEdit();

		public override void _Ready() {

			var label = new Label();
			label.Text = Name;
			AddChild(label);

			line.SizeFlagsHorizontal = (int)Control.SizeFlags.ExpandFill;

			line.Connect("text_entered", this, nameof(ValueEntered));
			line.Connect("focus_exited", this, nameof(ValueEntered));

			AddChild(line);
		}

		public void ValueEntered(string _) => ValueEntered();
		public abstract void ValueEntered();
	}

	public class StringControl : StringBase, IControl<string> {
		public event ValueSet<string> ValueSet;

		public override void ValueEntered() {
			ValueSet(line.Text);
		}
		public void Update(string value) => line.Text = value;
	}
}
