target("common")
    set_kind("static")
    add_files("*.cpp")

    add_includedirs("../include")
    add_includedirs("./")
    add_linkdirs("../lib")

    if is_plat("windows") then
        add_links("libuv")
        add_links("libboost_context-vc141-mt-sgd-x32-1_68")
        add_links("libboost_coroutine-vc141-mt-sgd-x32-1_68")
    else
        add_links("uv", "dl", "rt", "pthread", "boost_system", "boost_thread", "boost_context", "boost_coroutine");
    end

