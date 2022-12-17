#include "inspector.hpp"

#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"

class Widget : public WRL::EventHandler {
  protected:
	virtual void input_value(Variant value) = 0;
	void output_value(Variant value, bool commit) {
		WRL::Change::PropertyMap propertyChanges;
		propertyChanges.insert({field_key.first, field_key.second}, value);
		wrl->submit_change(WRL::Change{.propertyChanges = propertyChanges});
	}

  public:
	virtual void set_enabled(bool) = 0;
	Pair<WRL::EntryID, String> field_key;

  protected:
	void _wrl_changed(const WRL::Change& change, bool reset) override {
		if (reset) {
			if (wrl.is_valid()) {
				Variant value = wrl->get_entry_property(field_key.first, field_key.second);
				if (value.get_type() != Variant::Type::NIL) {
					input_value(value);
				}
			}
		}
		if (change.propertyChanges.has(field_key)) {
			input_value(change.propertyChanges.get(field_key));
		}
	}
	bool lite_init() override { return true; }
};

class StringWidget : public LineEdit, public Widget {
	GDCLASS(StringWidget, LineEdit)
  public:
	void set_enabled(bool enable) override { set_editable(enable); }

  protected:
	void input_value(Variant value) override { set_text(value); }

	void _text_submitted(String value) { output_value(value, true); }
	void _notification(int p_what) {
		if (p_what == NOTIFICATION_READY) {
			connect("text_submitted", callable_mp(this, &StringWidget::_text_submitted));
		}
	}
};

class IntWidget : public LineEdit, public Widget {
	GDCLASS(IntWidget, LineEdit)

  public:
	void set_enabled(bool enable) override { set_editable(enable); }

  protected:
	void input_value(Variant value) override { set_text(String::num_int64(value)); }

	void _text_submitted(String value) { output_value(value.to_int(), true); }
	void _notification(int p_what) {
		if (p_what == NOTIFICATION_READY) {
			connect("text_submitted", callable_mp(this, &IntWidget::_text_submitted));
		}
	}
};

class FloatWidget : public LineEdit, public Widget {
	GDCLASS(FloatWidget, LineEdit)

  public:
	void set_enabled(bool enable) override { set_editable(enable); }

  protected:
	void input_value(Variant value) override { set_text(String::num(value)); }

	void _text_submitted(String value) { output_value(value.to_float(), true); }
	void _notification(int p_what) {
		if (p_what == NOTIFICATION_READY) {
			connect("text_submitted", callable_mp(this, &FloatWidget::_text_submitted));
		}
	}
};

class Vector2Widget : public HBoxContainer, public Widget {
	GDCLASS(Vector2Widget, HBoxContainer)

	LineEdit* x;
	LineEdit* y;

  public:
	void set_enabled(bool enable) override {
		x->set_editable(enable);
		y->set_editable(enable);
	}

  protected:
	void input_value(Variant value) override {
		Vector2 vec = value;
		x->set_text(String::num(vec.x));
		y->set_text(String::num(vec.y));
	}

	void update_output(String) {
		output_value(
			Vector2{static_cast<real_t>(x->get_text().to_float()), static_cast<real_t>(y->get_text().to_float())},
			true);
	}
	void _notification(int p_what) {
		if (p_what == NOTIFICATION_READY) {
			x = memnew(LineEdit);
			x->set_h_size_flags(SIZE_EXPAND_FILL);
			x->connect("text_submitted", callable_mp(this, &Vector2Widget::update_output));
			add_child(x);
			y = memnew(LineEdit);
			y->set_h_size_flags(SIZE_EXPAND_FILL);
			y->connect("text_submitted", callable_mp(this, &Vector2Widget::update_output));
			add_child(y);
		}
	}
};

class Vector3Widget : public HBoxContainer, public Widget {
	GDCLASS(Vector3Widget, HBoxContainer)

	LineEdit* x;
	LineEdit* y;
	LineEdit* z;

  public:
	void set_enabled(bool enable) override {
		x->set_editable(enable);
		y->set_editable(enable);
		z->set_editable(enable);
	}

  protected:
	void input_value(Variant value) override {
		Vector3 vec = value;
		x->set_text(String::num(vec.x));
		y->set_text(String::num(vec.y));
		z->set_text(String::num(vec.z));
	}

	void update_output(String) {
		output_value(
			Vector3{
				static_cast<real_t>(x->get_text().to_float()), static_cast<real_t>(y->get_text().to_float()),
				static_cast<real_t>(z->get_text().to_float())},
			true);
	}
	void _notification(int p_what) {
		if (p_what == NOTIFICATION_READY) {
			x = memnew(LineEdit);
			x->set_h_size_flags(SIZE_EXPAND_FILL);
			x->connect("text_submitted", callable_mp(this, &Vector3Widget::update_output));
			add_child(x);
			y = memnew(LineEdit);
			y->set_h_size_flags(SIZE_EXPAND_FILL);
			y->connect("text_submitted", callable_mp(this, &Vector3Widget::update_output));
			add_child(y);
			z = memnew(LineEdit);
			z->set_h_size_flags(SIZE_EXPAND_FILL);
			z->connect("text_submitted", callable_mp(this, &Vector3Widget::update_output));
			add_child(z);
		}
	}
};

class RotationWidget : public HBoxContainer, public Widget {
	GDCLASS(RotationWidget, HBoxContainer)

	LineEdit* x;
	LineEdit* y;
	LineEdit* z;

  public:
	void set_enabled(bool enable) override {
		x->set_editable(enable);
		y->set_editable(enable);
		z->set_editable(enable);
	}

  protected:
	void input_value(Variant value) override {
		Quaternion quat = value;
		Vector3 vec = quat.get_euler() * 180 / Math_PI;
		x->set_text(String::num(vec.x));
		y->set_text(String::num(vec.y));
		z->set_text(String::num(vec.z));
	}

	void update_output(String) {
		output_value(
			Quaternion::from_euler(
				Vector3{
					static_cast<real_t>(x->get_text().to_float()), static_cast<real_t>(y->get_text().to_float()),
					static_cast<real_t>(z->get_text().to_float())} *
				Math_PI / 180),
			true);
	}
	void _notification(int p_what) {
		if (p_what == NOTIFICATION_READY) {
			x = memnew(LineEdit);
			x->set_h_size_flags(SIZE_EXPAND_FILL);
			x->connect("text_submitted", callable_mp(this, &RotationWidget::update_output));
			add_child(x);
			y = memnew(LineEdit);
			y->set_h_size_flags(SIZE_EXPAND_FILL);
			y->connect("text_submitted", callable_mp(this, &RotationWidget::update_output));
			add_child(y);
			z = memnew(LineEdit);
			z->set_h_size_flags(SIZE_EXPAND_FILL);
			z->connect("text_submitted", callable_mp(this, &RotationWidget::update_output));
			add_child(z);
		}
	}
};

void Inspector::_wrl_changed(const WRL::Change& change, bool) {
	if (change.select_changed) {
		if (vbox) {
			vbox->queue_free();
			vbox = nullptr;
		}
		selected = change.select.second;
		if (selected) {
			vbox = memnew(VBoxContainer);
			add_child(vbox);
			vbox->set_h_size_flags(SIZE_EXPAND_FILL);
			const WRL::Format& format = wrl->get_entry_format(selected);
			{
				Label* type_label = memnew(Label);
				type_label->set_text(format.type);
				type_label->add_theme_font_size_override(
					"font_size", type_label->get_theme_font_size("font_size") * 1.2);
				vbox->add_child(type_label);
			}

			Vector<WRL::Format::Property> properties;

			for (const auto& prop : format.properties) {
				if (prop.name.get(0) != '_') {
					properties.append(prop);
				}
			}
			for (const auto& prop : format.properties) {
				if (prop.name.get(0) == '_') {
					properties.append(prop);
				}
			}

			for (const auto& prop : properties) {
				Label* label = memnew(Label);
				label->set_text(prop.name);
				vbox->add_child(label);

				Widget* widget;
				switch (prop.type) {
				case Variant::STRING:
					widget = memnew(StringWidget);
					break;
				case Variant::INT:
					widget = memnew(IntWidget);
					break;
				case Variant::FLOAT:
					widget = memnew(FloatWidget);
					break;
				case Variant::VECTOR2:
					widget = memnew(Vector2Widget);
					break;
				case Variant::VECTOR3:
					widget = memnew(Vector3Widget);
					break;
				case Variant::QUATERNION:
					widget = memnew(RotationWidget);
					break;

				default:
					continue;
				}

				widget->field_key = {selected, prop.name};
				vbox->add_child(dynamic_cast<Control*>(widget));
				widget->wrl_connect(wrl);
			}
		}
	}
}
