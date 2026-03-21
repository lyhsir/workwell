// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "WorkWell",
    platforms: [
        .macOS(.v13)
    ],
    products: [
        .executable(
            name: "WorkWell",
            targets: ["WorkWell"]
        )
    ],
    dependencies: [],
    targets: [
        .executableTarget(
            name: "WorkWell",
            dependencies: [],
            resources: [
                .process("Timeout-16.png")
            ],
            swiftSettings: [
                .unsafeFlags(["-parse-as-library"])
            ]
        )
    ]
)
