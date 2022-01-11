
add_rules("mode.debug", "mode.release")

target("sc")
	before_build(function (target)
		--os.exec("sokol-shdc --input src/shaders/shaders.glsl --output src/shaders/shaders.h --slang glsl330", sourcefile, np)
	end)
	set_kind("binary")
	add_includedirs("src", "src/sc", "src/ext")
	add_cxxflags("-Wall")
	add_files("src/**.cpp")
	add_defines("USE_SHADER_PIPELINE=1")
	add_links("GL", "glfw")

