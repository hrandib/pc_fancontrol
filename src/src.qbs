import qbs
import qbs.FileInfo

Product { name: "src"

    Export {
        Depends { name: "cpp" }

        cpp.includePaths: [
            product.sourceDirectory
        ]

        Group { name: "common"
            condition: parent.name != "tests"
            prefix: "common/"
            files: [
                "constants.h",
                "algorithms.h",
                "algorithms.cpp",
            ]
            cpp.includePaths: outer.concat(FileInfo.joinPaths(product.sourceDirectory, "common"))
        }

        Group { name: "interface"
            prefix: "interface/"
            files: [
                "pwm.cpp",
                "pwm.h",
                "sensor.cpp",
                "sensor.h",
            ]
        }

        Group { name: "hwmon"
            prefix: "hwmon/"
            files: [
                "hwmon.cpp",
                "hwmon.h",
                "pwm_impl.cpp",
                "pwm_impl.h",
                "sensor_impl.cpp",
                "sensor_impl.h",
            ]
        }

        Group { name: "shell"
            prefix: "shell/"
            files: [
                "sensor_impl.cpp",
                "sensor_impl.h",
            ]
        }

        Group { name: "sysfs"
            prefix: "sysfs/"
            files: [
                "reader_impl.cpp",
                "reader_impl.h",
                "reader_writer.h",
                "reader_writer.cpp",
                "writer_impl.cpp",
                "writer_impl.h",
            ]
        }
    }
}
