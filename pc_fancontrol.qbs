import qbs

Project {

CppApplication { name: "fancontrol"

    cpp.optimization: "fast"
    cpp.debugInformation: false
    cpp.includePaths: [
        "src"
    ]
    cpp.cFlags: [
        "-std=c17"
    ]
    cpp.cxxFlags: [
        "-std=c++2a"
    ]
    cpp.commonCompilerFlags: [
        "-Wall", "-Wextra"
    ]

    Group { name: "source"
        prefix: "src/"
        files: [
            "hwmon.cpp",
            "hwmon.h",
            "main.cpp",
        ]
    }

} //CppApplication

} //Project
