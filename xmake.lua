
add_rules("mode.debug", "mode.release")

rule("glsl")
	set_extensions(".glsl")
	on_build_file(function (target, sourcefile)
		local np = path.join(path.directory(sourcefile), path.basename(sourcefile) .. ".h")
		os.exec("sokol-shdc --input %s --output %s --slang glsl330", sourcefile, np)
	end)

target("sc")
	add_rules("glsl")
	set_kind("binary")
	add_includedirs("src", "src/sc", "src/ext")
	add_cxxflags("-Wall")
	add_files("src/**.cpp")
	add_files("src/shaders/shaders.glsl")
	add_defines("USE_SHADER_PIPELINE=1")
	add_links("GL", "glfw")
