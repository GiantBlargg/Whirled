#![feature(proc_macro_span)]

extern crate proc_macro;
use proc_macro::{Span, TokenStream};
use quote::quote;
use shaderc::ShaderKind;

#[proc_macro]
pub fn compile_shader(token_stream: TokenStream) -> TokenStream {
	let path_raw = syn::parse_macro_input!(token_stream as syn::LitStr).value();

	let mut path = Span::call_site().source_file().path();
	path.pop();
	path.push(&path_raw);
	path = path.canonicalize().unwrap();

	let path_str = path.to_str().unwrap();

	let glsl = std::fs::read_to_string(&path).unwrap();

	let ascii_lowercase = path.extension().unwrap().to_ascii_lowercase();
	let extension = ascii_lowercase.to_str().unwrap();

	let shader_kind = match extension {
		"vert" => ShaderKind::Vertex,
		"frag" => ShaderKind::Fragment,
		"tesc" => ShaderKind::TessControl,
		"tese" => ShaderKind::TessEvaluation,
		"geom" => ShaderKind::Geometry,
		"comp" => ShaderKind::Compute,
		_ => panic!(),
	};

	let mut compiler = shaderc::Compiler::new().unwrap();

	let compiled = compiler
		.compile_into_spirv(&glsl, shader_kind, path_str, "main", Default::default())
		.unwrap();

	let spirv = compiled.as_binary();
	let spirv_len = spirv.len();

	let output = quote! {{
		std::include_bytes!(#path_str);
		[#(#spirv),*] as [u32; #spirv_len]
	}};
	output.into()
}
