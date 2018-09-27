set_project("userver")
set_optimize("none")
-- add modes: debug and release
add_rules("mode.debug", "mode.release")
set_languages("cxx14")

includes("common/xmake.lua")
includes("agentsvr/xmake.lua")
includes("loginsvr/xmake.lua")
