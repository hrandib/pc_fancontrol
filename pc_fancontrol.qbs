import qbs

Project {
    references: [
        "src/src.qbs",
        "src/yaml-cpp.qbs",
        "test/test.qbs"
    ]

CppApplication { name: "fancontrol"

    Depends { name: "yaml-cpp" }
    Depends { name: "src" }

    consoleApplication: true
    cpp.optimization: undefined
    cpp.debugInformation: false

    cpp.cFlags: [
        "-std=c17"
    ]
    cpp.cxxFlags: [
        "-std=c++2a"
    ]
    cpp.commonCompilerFlags: [
        "-O3", "-Wall", "-Wextra"
    ]

    Group { name: "main"
        prefix: "src/"
        files: [
            "controller.cpp",
            "controller.h",
            "configentry.h",
            "config.cpp",
            "config.h",
            "main.cpp",
        ]
    }

    Group { name: "resource"
        files: [
            "config/fancontrol.yaml"
        ]
    }

} //CppApplication

CppApplication { name: "tests"
    type: base.concat("autotest")
    condition: false
    builtByDefault: false

    Depends { name: "test" }
}


AutotestRunner { }
} //Project
