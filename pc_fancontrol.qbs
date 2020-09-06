import qbs

Project {
    references: [
        "src/src.qbs",
        "src/yaml-cpp.qbs",
        "test/test.qbs"
    ]

CppApplication { name: "fancontrol"

    Depends { name: "src" }
    Depends { name: "yaml-cpp" }

    consoleApplication: true

    cpp.cFlags: [
        "-std=c17"
    ]
    cpp.cxxFlags: [
        "-std=c++2a"
    ]
    cpp.commonCompilerFlags: [
        "-Wall", "-Wextra"
    ]

    Group { name: "main"
        prefix: "src/"
        files: [
            "main.cpp"
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
