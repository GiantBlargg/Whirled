using Godot;
using System;

namespace Controls {
	public class Number<T> : StringBase, IControl<T> where T : IConvertible {
		public event ValueSet<T> ValueSet;

		public override void ValueEntered() {
			var value = Parse(line.Text);
			ValueSet(value);
		}
		public void Update(T value) => line.Text = value.ToString();

		T Parse(string v) {
			return (T)Convert.ChangeType(v, typeof(T));
		}
	}
}