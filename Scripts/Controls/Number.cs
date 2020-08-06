using Godot;
using System;

namespace Controls {
	public class Number<T> : HBoxContainer, IControl<T> where T : IConvertible {
		public event ValueSet<T> ValueSet;

		LineEdit line = new LineEdit();

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
		public void ValueEntered() {
			var value = Parse(line.Text);
			ValueSet(value);
		}
		public void Update(T value) => line.Text = value.ToString();

		T Parse(string v) {
			return (T)Convert.ChangeType(v, typeof(T));
		}
	}
}