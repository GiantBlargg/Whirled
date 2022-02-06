#pragma once

#include "../wrl/wrl.hpp"
#include "scene/gui/box_container.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/scroll_container.h"
#include <functional>

class Inspector : public ScrollContainer, public WRL::EventHandler {
	GDCLASS(Inspector, ScrollContainer)

  private:
	WRL::EntryID selected;

	template <typename T> class Widget {
	  public:
		virtual void input_value(T value) = 0;
		std::function<void(T, bool)> output_value;

		virtual void set_enabled(bool) = 0;
	};

	class StringWidget : public LineEdit, public Widget<String> {
		GDCLASS(StringWidget, LineEdit)

	  public:
		void input_value(String value) override { set_text(value); }
		void set_enabled(bool enable) override { set_editable(enable); }

	  protected:
		void _text_submitted(String value) { output_value(value, true); }
		void _notification(int p_what) {
			if (p_what == NOTIFICATION_READY) {
				connect("text_submitted", callable_mp(this, &StringWidget::_text_submitted));
			}
		}
	};

	class IntWidget : public LineEdit, public Widget<int> {
		GDCLASS(IntWidget, LineEdit)

	  public:
		void input_value(int value) override { set_text(String::num_int64(value)); }
		void set_enabled(bool enable) override { set_editable(enable); }

	  protected:
		void _text_submitted(String value) { output_value(value.to_int(), true); }
		void _notification(int p_what) {
			if (p_what == NOTIFICATION_READY) {
				connect("text_submitted", callable_mp(this, &IntWidget::_text_submitted));
			}
		}
	};

	class FloatWidget : public LineEdit, public Widget<float> {
		GDCLASS(FloatWidget, LineEdit)

	  public:
		void input_value(float value) override { set_text(String::num(value)); }
		float get_float_value() { return get_text().to_float(); }
		void set_enabled(bool enable) override { set_editable(enable); }

	  protected:
		void _text_submitted(String value) { output_value(value.to_float(), true); }
		void _notification(int p_what) {
			if (p_what == NOTIFICATION_READY) {
				connect("text_submitted", callable_mp(this, &FloatWidget::_text_submitted));
			}
		}
	};

	class Vector2Widget : public HBoxContainer, public Widget<Vector2> {
		GDCLASS(Vector2Widget, HBoxContainer)

		FloatWidget* x;
		FloatWidget* y;

	  public:
		void input_value(Vector2 value) override {
			x->input_value(value.x);
			y->input_value(value.y);
		}
		void set_enabled(bool enable) override {
			x->set_editable(enable);
			y->set_editable(enable);
		}

	  private:
		void update_output() { output_value(Vector2{x->get_float_value(), y->get_float_value()}, true); }

	  public:
		Vector2Widget() {
			x = memnew(FloatWidget);
			x->set_h_size_flags(SIZE_EXPAND_FILL);
			x->output_value = [this](float, bool) { update_output(); };
			add_child(x);
			y = memnew(FloatWidget);
			y->set_h_size_flags(SIZE_EXPAND_FILL);
			y->output_value = [this](float, bool) { update_output(); };
			add_child(y);
		}
	};

	class Vector3Widget : public HBoxContainer, public Widget<Vector3> {
		GDCLASS(Vector3Widget, HBoxContainer)

		FloatWidget* x;
		FloatWidget* y;
		FloatWidget* z;

	  public:
		void input_value(Vector3 value) override {
			x->input_value(value.x);
			y->input_value(value.y);
			z->input_value(value.z);
		}
		void set_enabled(bool enable) override {
			x->set_editable(enable);
			y->set_editable(enable);
			z->set_editable(enable);
		}

	  private:
		void update_output() {
			output_value(Vector3{x->get_float_value(), y->get_float_value(), z->get_float_value()}, true);
		}

	  public:
		Vector3Widget() {
			x = memnew(FloatWidget);
			x->set_h_size_flags(SIZE_EXPAND_FILL);
			x->output_value = [this](float, bool) { update_output(); };
			add_child(x);
			y = memnew(FloatWidget);
			y->set_h_size_flags(SIZE_EXPAND_FILL);
			y->output_value = [this](float, bool) { update_output(); };
			add_child(y);
			z = memnew(FloatWidget);
			z->set_h_size_flags(SIZE_EXPAND_FILL);
			z->output_value = [this](float, bool) { update_output(); };
			add_child(z);
		}
	};

	class RotationWidget : public HBoxContainer, public Widget<Quaternion> {
		GDCLASS(RotationWidget, HBoxContainer)

		FloatWidget* x;
		FloatWidget* y;
		FloatWidget* z;

	  public:
		void input_value(Quaternion value) override {
			Vector3 euler = value.get_euler() * (180 / Math_PI);
			x->input_value(euler.x);
			y->input_value(euler.y);
			z->input_value(euler.z);
		}
		void set_enabled(bool enable) override {
			x->set_editable(enable);
			y->set_editable(enable);
			z->set_editable(enable);
		}

	  private:
		void update_output() {
			output_value(
				Quaternion(Vector3{x->get_float_value(), y->get_float_value(), z->get_float_value()} * Math_PI / 180),
				true);
		}

	  public:
		RotationWidget() {
			x = memnew(FloatWidget);
			x->set_h_size_flags(SIZE_EXPAND_FILL);
			x->output_value = [this](float, bool) { update_output(); };
			add_child(x);
			y = memnew(FloatWidget);
			y->set_h_size_flags(SIZE_EXPAND_FILL);
			y->output_value = [this](float, bool) { update_output(); };
			add_child(y);
			z = memnew(FloatWidget);
			z->set_h_size_flags(SIZE_EXPAND_FILL);
			z->output_value = [this](float, bool) { update_output(); };
			add_child(z);
		}
	};

	VBoxContainer* vbox;

  protected:
	void _wrl_event(const WRL::Event&) override;

  private:
#include "../wrl/declare_widgets.gen.ipp"
#include "../wrl/init_widgets.gen.ipp"
#include "../wrl/update_widgets.gen.ipp"
};