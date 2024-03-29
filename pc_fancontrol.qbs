import qbs

Project {

    minimumQbsVersion: "1.22.0"

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

    cpp.cFlags: [
        "-std=c17"
    ]
    cpp.cxxFlags: [
        "-std=c++20"
    ]
    cpp.commonCompilerFlags: [
        "-Wall", "-Wextra"
    ]
    cpp.driverFlags: [
        "-flto=auto", "-O3"
    ]
    cpp.dynamicLibraries: [
        "pthread"
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
            "main.h",
        ]
    }

    Group { name: "resource"
        prefix: "config/"
        files: [
            "fancontrol.yaml",
            "fancontrol.service",
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
