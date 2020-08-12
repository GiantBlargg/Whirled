using System;
using System.Linq.Expressions;

public abstract class ObjectProperty {
	public virtual string Name { get; protected set; }
	public abstract Type Type { get; }
}

public class ObjectProperty<T> : ObjectProperty {
	Func<T> getter;
	Action<T> setter;

	public override Type Type => typeof(T);

	public ObjectProperty(Expression<Func<T>> propertyExpression, string name = null) :
		this(propertyExpression.Body, name) { }

	public ObjectProperty(Expression propertyExpression, string name = null) {
		Name = name ?? DetermineName(propertyExpression as MemberExpression);
		getter = GenerateGetter(propertyExpression);
		setter = GenerateSetter(propertyExpression);
	}

	static string DetermineName(MemberExpression propertyExpression) => propertyExpression.Member.Name;

	static Func<T> GenerateGetter(Expression propertyExpression) {
		var getter = Expression.Lambda<Func<T>>(propertyExpression);
		return getter.Compile();
	}

	static Action<T> GenerateSetter(Expression propertyExpression) {
		var value = Expression.Parameter(typeof(T));
		var assign = Expression.Assign(propertyExpression, value);
		var setter = Expression.Lambda<Action<T>>(assign, value);
		return setter.Compile();
	}

	public T Get() => getter();
	public void Set(T t, bool updateHistory = true) {
		setter(t);
		Update(t);
	}

	public event Action<T> Update;

	public T Value { get => Get(); set => setter(value); }
}
