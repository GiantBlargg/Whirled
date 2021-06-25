using Godot;
using System;
using System.Linq;

namespace Controls {
	public partial class ArrayControl<T> : VBoxContainer, IControl<T[]> {

		IControl<T>[] controls;

		public ArrayControl(int length, Func<IControl<T>> makeControl) {
			controls = new IControl<T>[length];

			for (int i = 0; i < controls.Length; i++) {
				IControl<T> control = makeControl();

				control.ValueSet += ValueSet;

				controls[i] = control;
			}
		}

		public override void _Ready() {
			if (Name != "" && ((string)Name)[0] != '@') {
				var label = new Label();
				label.Text = Name;
				AddChild(label);
			}

			Array.ForEach(controls, control => {
				if (control is Node)
					AddChild(control as Node);
			});
		}

		public event ValueSet ValueSet;
		public T[] Value {
			get => controls.Select(c => c.Value).ToArray();
			set {
				for (int i = 0; i < value.Length; i++) {
					controls[i].Value = value[i];
				}
			}
		}
	}
}
