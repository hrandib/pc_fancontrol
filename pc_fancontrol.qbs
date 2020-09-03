import qbs

Project {
    references: [
        "src/src.qbs",
        "test/test.qbs"
    ]

CppApplication { name: "fancontrol"

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
    condition: false
    type: base.concat("autotest")
}


AutotestRunner { }
} //Project
