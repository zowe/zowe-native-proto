module.exports = {
    branches: [
        {
            name: "ci/fix-release-workflow",
            level: "minor"
        }
    ],
    plugins: [
        [
            "@octorelease/changelog",
            {
                headerLine: "## Recent Changes",
            },
            {
                $cwd: "native",
                headerLine: "## Recent Changes",
            },
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
                assets: ["dist/*.pax.Z", "dist/*.tgz", "dist/*.vsix"],
            },
        ],
        "@octorelease/git",
    ]
};
