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

	public ObjectProperty(Expression<Func<T>> propertyExpression) :
		this(propertyExpression.Body as MemberExpression) { }


	public ObjectProperty(MemberExpression propertyExpression, string name = null) {
		Name = name == null ? DetermineName(propertyExpression) : name;
		getter = GenerateGetter(propertyExpression);
		setter = GenerateSetter(propertyExpression);
	}

	static string DetermineName(MemberExpression propertyExpression) => propertyExpression.Member.Name;

	static Func<T> GenerateGetter(MemberExpression propertyExpression) {
		var getter = Expression.Lambda<Func<T>>(propertyExpression);
		return getter.Compile();
	}

	static Action<T> GenerateSetter(MemberExpression propertyExpression) {
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
