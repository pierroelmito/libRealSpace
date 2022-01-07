
add_rules("mode.debug", "mode.release")

rule("glsl")
	set_extensions(".glsl")
	on_build(function (target, sourcefile)
		--os.cp(sourcefile, path.join(target:targetdir(), path.basename(sourcefile) .. ".html"))
		 --input shaders.glsl --output shaders.h --slang glsl330
		--os.runv("~/bin/sokol-shdc", { "--input", sourcefile, "--output",, "--slang", "glsl330" })
	end)

target("sc")
	set_kind("binary")
	add_includedirs("src", "src/sc", "src/ext")
	add_cxxflags("-Wall")
	add_files("src/**.cpp")
	--add_files("src/shaders/shaders.glsl")
	add_defines("USE_SHADER_PIPELINE=1")
	add_links("GL", "glfw")
