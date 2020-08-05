using Godot;
using System;

namespace Controls {
	public class Number : HBoxContainer, IControl {
		public event ValueSet ValueSet;

		LineEdit line = new LineEdit();

		Type type;

		public Number(Type type) {
			this.type = type;
		}

		public override void _Ready() {

			line.SizeFlagsHorizontal = (int)Control.SizeFlags.ExpandFill;

			line.Connect("text_entered", this, nameof(ValueEntered));
			line.Connect("focus_exited", this, nameof(ValueEntered));

			AddChild(line);
		}

		public void ValueEntered() {
			var value = Parse(line.Text);
			ValueSet(value);
		}

		public void Update(object value) => Update((IConvertible)value);
		public void Update(IConvertible value) => line.Text = value.ToString();

		IConvertible Parse(string v) {
			return (IConvertible)Convert.ChangeType(v, type);
		}
	}
}