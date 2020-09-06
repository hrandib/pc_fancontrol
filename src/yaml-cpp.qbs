import qbs
import qbs.FileInfo

Product { name: "yaml-cpp"
    Export {
        Depends { name: "cpp" }

        cpp.includePaths: [
            "yaml-cpp/include"
        ]

        Group { name: "inc"
            prefix: "yaml-cpp/"
            files: [
                "include/yaml-cpp/**/*.h",
                "src/**/*.h"
            ]
        }
        Group { name: "src"
            prefix: "yaml-cpp/src/"
            files: [
                "**/*.cpp"
            ]
        }
    }
}
