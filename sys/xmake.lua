set_languages("c11")
set_optimize("fastest")
set_symbols("debug")

target("quickjs")
    set_kind("static")
    add_files("c-src/cutils.c")
    add_files("c-src/libbf.c")
    add_files("c-src/core/*.c")
    add_files("c-src/core/builtins/*.c")
    add_files("c-src/anode/*.c")
    add_includedirs("c-include")
    add_includedirs("c-src")
    