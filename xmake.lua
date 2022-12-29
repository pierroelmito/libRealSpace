
add_rules("mode.debug", "mode.release")

target("sc")
	before_build(function (target)
		-- uncomment do rebuild shaders from glsl:
		os.exec("sokol-shdc --input src/shaders/shaders.glsl --output src/shaders/shaders.h --slang glsl330", sourcefile, np)
	end)
	--set_optimize("fast")
	set_kind("binary")
	add_includedirs("src", "src/sc", "src/ext")
	--add_cxxflags("-Wall")
	add_cxxflags("--std=c++20")
	add_files("src/**.cpp")
	add_defines("USE_SHADER_PIPELINE=1")
	if is_plat("mingw") then
		add_links("opengl32", "ole32", "glfw3")
	else
		add_links("GL", "glfw", "pthread", "asound")
	end

