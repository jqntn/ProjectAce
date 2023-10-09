set_project("MyProjectName")

add_requires("enet6 >=6.0.2")

add_rules("mode.asan", "mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

set_allowedmodes("debug", "releasedbg", "asan")
set_allowedplats("windows", "mingw", "linux", "macosx")
set_allowedarchs("windows|x64", "mingw|x86_64", "linux|x86_64", "macosx|x86_64")
set_defaultmode("debug")

add_includedirs("src")
add_sysincludedirs("thirdparty/include")

set_targetdir("./bin/$(os)_$(arch)_$(mode)")
set_languages("c89", "cxx17")
set_runtimes(is_mode("debug") and "MDd" or "MD")
set_warnings("allextra")

if is_mode("release") then
	set_fpmodels("fast")
	add_vectorexts("sse", "sse2", "sse3", "ssse3")
end

target("NetworkServer")
    set_kind("binary")

	add_headerfiles("src/**.hpp", "src/**.inl")
	add_files("src/**.cpp")
	add_packages("enet6")
