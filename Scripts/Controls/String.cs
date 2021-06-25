using Godot;
using System;

namespace Controls {
	public abstract partial class StringBase : HBoxContainer {

		protected LineEdit line = new LineEdit();

		public override void _Ready() {

			if (Name != "" && ((string)Name)[0] != '@') {
				var label = new Label();
				label.Text = Name;
				AddChild(label);
			}

			line.SizeFlagsHorizontal = (int)Control.SizeFlags.ExpandFill;

			line.TextSubmitted += ValueEntered;
			line.FocusExited += ValueEntered;

			AddChild(line);
		}

		public event ValueSet ValueSet;

		public void ValueEntered(string _) => ValueSet();
		public void ValueEntered() => ValueSet();
	}

	public partial class StringControl : StringBase, IControl<string> {
		public string Value {
			get => line.Text;
			set => line.Text = value;
		}
	}

	public partial class Number<T> : StringBase, IControl<T> where T : IConvertible {
		public T Value {
			get => (T)Convert.ChangeType(line.Text, typeof(T));
			set => line.Text = value.ToString();
		}
	}
}
