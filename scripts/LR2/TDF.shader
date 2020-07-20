shader_type spatial;
render_mode blend_mix,depth_draw_opaque,cull_back,diffuse_lambert,specular_disabled,vertex_lighting;

uniform sampler2D tex0: hint_black_albedo;
uniform sampler2D tex1: hint_black_albedo;
uniform sampler2D tex2: hint_black_albedo;
uniform sampler2D tex3: hint_black_albedo;

void fragment() {

	ALBEDO = 
		COLOR.r * texture(tex0, UV).rgb +
		COLOR.g * texture(tex1, UV).rgb +
		COLOR.b * texture(tex2, UV).rgb +
		COLOR.a * texture(tex3, UV).rgb;
}