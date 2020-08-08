using Godot;
using System;

namespace Controls {
	public abstract class StringBase : HBoxContainer {

		protected LineEdit line = new LineEdit();

		public override void _Ready() {

			if (Name != "" && Name[0] != '@') {
				var label = new Label();
				label.Text = Name;
				AddChild(label);
			}

			line.SizeFlagsHorizontal = (int)Control.SizeFlags.ExpandFill;

			line.Connect("text_entered", this, nameof(ValueEntered));
			line.Connect("focus_exited", this, nameof(ValueEntered));

			AddChild(line);
		}

		public event ValueSet ValueSet;

		public void ValueEntered(string _) => ValueSet();
		public void ValueEntered() => ValueSet();
	}

	public class StringControl : StringBase, IControl<string> {
		public string Value {
			get => line.Text;
			set => line.Text = value;
		}
	}

	public class Number<T> : StringBase, IControl<T> where T : IConvertible {
		public T Value {
			get => (T)Convert.ChangeType(line.Text, typeof(T));
			set => line.Text = value.ToString();
		}
	}
}
