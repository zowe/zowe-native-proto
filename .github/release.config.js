module.exports = {
    branches: [
        {
            name: "main",
            level: "minor"
        }
    ],
    plugins: [
        [
            "@octorelease/changelog",
            {
                displayNames: {
                    "vsce": "VS Code Extension",
                    "cli": "CLI Plug-in",
                    "sdk": "Client SDK",
                    "native": "z/OS Components"
                },
                extraDirs: ["native"],
                headerLine: "## Recent Changes",
            }
        ],
        [
            "@octorelease/lerna",
            {
                // Use Lerna only for versioning and publish packages independently
                npmPublish: false,
            },
        ],
        [
            "@octorelease/exec",
            {
                $cwd: "packages/cli",
                dryRunAllow: ["publish"],
                publishCmd: "npm run package",
            },
        ],
        // TODO Replace exec plugin with the following once we're ready to publish to npm
        // [
        //     "@octorelease/npm",
        //     {
        //         $cwd: "packages/cli",
        //         npmPublish: false,
        //         tarballDir: "dist",
        //     },
        // ],
        [
            "@octorelease/vsce",
            {
                $cwd: "packages/vsce",
                vscePublish: false,
                vsixDir: "dist",
            },
        ],
        [
            "@octorelease/github",
            {
                assets: process.env.TRAIN == "Release" ? ["dist/*.pax.Z", "dist/*.tgz", "dist/*.vsix"] : []
            },
        ],
        "@octorelease/git",
    ]
};
