use std::{env, ffi::OsString, path::PathBuf};

fn stderr_ok<T, E: std::fmt::Debug>(result: Result<T, E>) -> Option<T> {
	match result {
		Ok(t) => Some(t),
		Err(err) => {
			eprintln!("{:?}", err);
			None
		}
	}
}

fn main() {
	let shader_types = {
		use shaderc::ShaderKind;

		let mut shader_type = std::collections::HashMap::new();

		shader_type.insert(OsString::from("vert"), ShaderKind::Vertex);
		shader_type.insert(OsString::from("frag"), ShaderKind::Fragment);
		shader_type.insert(OsString::from("tesc"), ShaderKind::TessControl);
		shader_type.insert(OsString::from("tese"), ShaderKind::TessEvaluation);
		shader_type.insert(OsString::from("geom"), ShaderKind::Geometry);
		shader_type.insert(OsString::from("comp"), ShaderKind::Compute);

		shader_type
	};

	let mut compiler = shaderc::Compiler::new().unwrap();
	let options = {
		let mut options = shaderc::CompileOptions::new().unwrap();
		options.set_target_env(
			shaderc::TargetEnv::Vulkan,
			shaderc::EnvVersion::Vulkan1_2 as u32,
		);
		options
	};

	let src_dir = PathBuf::from(env::var_os("CARGO_MANIFEST_DIR").unwrap()).join("src");
	let out_dir = PathBuf::from(env::var_os("OUT_DIR").unwrap());
	let output = walkdir::WalkDir::new(&src_dir)
		.into_iter()
		.filter_map(|e| {
			let entry = stderr_ok(e)?;
			if !stderr_ok(entry.metadata())?.is_file() {
				return None;
			}
			let in_file = entry.into_path();
			let shader_type = {
				let extension = in_file.extension()?.to_ascii_lowercase();
				*shader_types.get(&extension)?
			};
			Some((in_file, shader_type))
		})
		.map(|(in_file, shader_type)| {
			println!("cargo:rerun-if-changed={}", in_file.to_str().unwrap());

			let glsl = std::fs::read_to_string(&in_file).unwrap();
			let spirv: Vec<u32> = compiler
				.compile_into_spirv(
					&glsl,
					shader_type,
					in_file.to_str().unwrap(),
					"main",
					Some(&options),
				)
				.unwrap()
				.as_binary()
				.into();
			let relative_path = in_file.strip_prefix(&src_dir).unwrap();
			// let out_file = {
			// 	let mut out_file = out_dir.join(relative_path);
			// 	let mut extension = out_file.extension().unwrap().to_os_string();
			// 	extension.push(".spv");
			// 	out_file.set_extension(extension);
			// 	out_file
			// };
			let shader_name = relative_path
				.to_str()
				.unwrap()
				.split(|c: char| !c.is_ascii_alphanumeric())
				.filter(|s| *s != "")
				.collect::<Vec<_>>()
				.join("_")
				.to_ascii_uppercase();
			(spirv, shader_name)
		})
		.map(|(spirv, shader_name)| {
			format!(
				"pub const {}: [u32; {}] = {:?};",
				shader_name,
				spirv.len(),
				spirv
			)
		})
		.collect::<Vec<_>>()
		.join("\n");
	std::fs::write(out_dir.join("shaders.rs"), output).unwrap();
}
