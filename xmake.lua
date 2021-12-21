
add_rules("mode.debug", "mode.release")

target("sc")
	set_kind("binary")
	add_files("src/**.cpp")
	add_links("GL", "SDL2")

