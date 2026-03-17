# VS Code Extension — AI Agent Guide

## Icon Generation

The extension icon (`icon.png`) is generated programmatically using `scripts/generateIcon.ts`.

The script uses the `canvas` npm package to:

1. Download the original Zowe Explorer color icon from the `zowe/zowe-explorer-vscode` GitHub repo
2. Overlay an SSH terminal prompt badge (`>_`) in the bottom-right corner
3. Export a 256x256 PNG to `packages/vsce/icon.png`

### Regenerating the icon

```sh
npx tsx scripts/generateIcon.ts
```

### Customization

- **Badge size**: Adjust `badgeR` (radius as fraction of `SIZE`)
- **Badge position**: Adjust `badgeCx` / `badgeCy`
- **Badge colors**: Change `ACCENT_CYAN` (prompt symbols) or the gradient stops in `badgeGrad`
- **Output size**: Change the `SIZE` constant (default 256)

### Related files

- `package.json` — references the icon via `"icon": "icon.png"`
- `.vscodeignore` — allowlists `icon.png` for VSIX packaging
