project "ocgcore"
    kind "StaticLib"

    files { "**.cc", "**.cpp", "**.c", "**.h" }
    configuration "windows"
        includedirs { ".." }
    configuration "not vs*"
        buildoptions { "-std=c++1y" }
    configuration "not windows"
        includedirs { "/usr/include/lua", "/usr/include/lua5.3", "/usr/include/lua/5.3" }
