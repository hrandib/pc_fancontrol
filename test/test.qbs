import qbs

Product { name: "test"

    Export {
        Depends { name: "cpp" }
    }
}
