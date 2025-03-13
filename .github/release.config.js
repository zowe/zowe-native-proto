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
            "@octorelease/npm",
            {
                $cwd: "packages/cli",
                npmPublish: false,
                tarballDir: "dist",
            },
        ],
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
