target("agentsvr")
    set_kind("binary")
    add_deps("common")

    add_includedirs("../include")
    add_includedirs("./")
    add_includedirs("../common")
    add_files("*.cpp")
