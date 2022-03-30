import qbs
import qbs.FileInfo

Product { name: "yaml-cpp"

    Export {
        Depends { name: "cpp" }

        cpp.includePaths: [
            FileInfo.joinPaths(exportingProduct.sourceDirectory, "yaml-cpp/include")
        ]

        Group { name: "yaml-inc"
            prefix: "yaml-cpp/"
            files: [
                "include/yaml-cpp/**/*.h",
                "src/**/*.h"
            ]
        }
        Group { name: "yaml-src"
            prefix: "yaml-cpp/src/"
            files: [
                "**/*.cpp"
            ]
        }
    }
}
