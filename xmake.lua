
add_rules("mode.debug", "mode.release")

target("sc")
	set_kind("binary")
	add_includedirs("src", "src/sc", "src/ext")
	add_files("src/**.cpp")
	add_defines("USE_SHADER_PIPELINE=1")
	add_links("GL", "glfw")
