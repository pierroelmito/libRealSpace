
add_rules("mode.debug", "mode.release")

rule("shader")
	set_extensions(".args")
	on_buildcmd_file(function (target, batchcmds, sourcefile, opt)
		batchcmds:mkdir(target:targetdir())
		local dir = path.directory(sourcefile)
		local src = path.filename(sourcefile)
		local dst = path.basename(sourcefile) .. ".glsl"
		local targetfile = path.join(dir, dst)
		local global = path.join(dir, "global.shader")
		local common = path.join(dir, "common.glsl")
		batchcmds:vrunv('/bin/sh', {'-c', "cd " .. dir .. "; mcpp -k `cat " .. src .. "` -P global.shader | grep -v '^$' > " .. dst})
		batchcmds:show_progress(opt.progress, "${color.build.object}shader %s", targetfile)
		batchcmds:add_depfiles(sourcefile, global, common)
	end)

target("sc")
	add_rules("shader")
	set_kind("binary")
	add_includedirs("src", "src/sc", "src/ext")
	add_cxxflags("--std=c++20")
	add_files("src/**.cpp")
	add_files("data/shaders/*.args")
	add_defines("USE_SHADER_PIPELINE=1")
	add_links("raylib")
	set_rundir("data")
	--[[
	if is_plat("mingw") then
		add_links("opengl32", "ole32", "glfw3")
	else
		add_links("GL", "glfw", "pthread", "asound")
	end
	]]--

