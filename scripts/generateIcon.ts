/**
 * Icon generator for the Zowe Remote SSH VS Code extension.
 *
 * Downloads the original Zowe Explorer color icon (the blue diamond with
 * white "Z") and overlays an SSH terminal prompt badge (">_") in the
 * bottom-right corner to distinguish this extension as the Remote SSH variant.
 *
 * Uses the `canvas` npm package (node-canvas).
 *
 * Usage:
 *   npx tsx scripts/generateIcon.ts
 *
 * Output:
 *   packages/vsce/icon.png
 */

import { mkdirSync, writeFileSync } from "node:fs";
import { dirname, resolve } from "node:path";
import { createCanvas, loadImage } from "canvas";

// --- Configuration -----------------------------------------------------------

const SIZE = 256;
const OUTPUT = resolve(__dirname, "..", "packages", "vsce", "icon.png");

// URL of the original Zowe Explorer color icon (256×256 PNG).
const ZOWE_ICON_URL =
    "https://raw.githubusercontent.com/zowe/zowe-explorer-vscode/main/packages/zowe-explorer/resources/zowe-icon-color.png";

// Badge colours
const WHITE = "#FFFFFF";
const ACCENT_CYAN = "#4FC3F7"; // light cyan for the terminal prompt symbols

// --- Main --------------------------------------------------------------------

async function main() {
    const canvas = createCanvas(SIZE, SIZE);
    const ctx = canvas.getContext("2d");

    // 1. Load & draw the original Zowe Explorer icon --------------------------
    const baseIcon = await loadImage(ZOWE_ICON_URL);
    ctx.drawImage(baseIcon, 0, 0, SIZE, SIZE);

    // 2. SSH terminal prompt badge --------------------------------------------
    //    A small circle in the bottom-right with ">_" inside.

    const badgeR = SIZE * 0.19;
    const badgeCx = SIZE - badgeR - 4;
    const badgeCy = SIZE - badgeR - 4;

    // Dark blue background circle
    const badgeGrad = ctx.createRadialGradient(badgeCx, badgeCy, 0, badgeCx, badgeCy, badgeR);
    badgeGrad.addColorStop(0, "#0D47A1");
    badgeGrad.addColorStop(1, "#1565C0");
    ctx.beginPath();
    ctx.arc(badgeCx, badgeCy, badgeR, 0, Math.PI * 2);
    ctx.closePath();
    ctx.fillStyle = badgeGrad;
    ctx.fill();

    // White border ring for separation from the base icon
    ctx.beginPath();
    ctx.arc(badgeCx, badgeCy, badgeR, 0, Math.PI * 2);
    ctx.closePath();
    ctx.lineWidth = 3;
    ctx.strokeStyle = WHITE;
    ctx.stroke();

    // ">" prompt chevron
    ctx.save();
    ctx.translate(badgeCx, badgeCy);

    const ps = badgeR * 0.55; // prompt scale factor

    ctx.beginPath();
    ctx.moveTo(-ps * 0.6, -ps * 0.55);
    ctx.lineTo(ps * 0.1, 0);
    ctx.lineTo(-ps * 0.6, ps * 0.55);
    ctx.lineWidth = 3.5;
    ctx.strokeStyle = ACCENT_CYAN;
    ctx.lineCap = "round";
    ctx.lineJoin = "round";
    ctx.stroke();

    // "_" cursor line
    ctx.beginPath();
    ctx.moveTo(ps * 0.25, ps * 0.55);
    ctx.lineTo(ps * 0.75, ps * 0.55);
    ctx.lineWidth = 3.5;
    ctx.strokeStyle = ACCENT_CYAN;
    ctx.lineCap = "round";
    ctx.stroke();

    ctx.restore();

    // 3. Export ----------------------------------------------------------------
    mkdirSync(dirname(OUTPUT), { recursive: true });
    const buffer = canvas.toBuffer("image/png");
    writeFileSync(OUTPUT, buffer);

    console.log(`Icon written to ${OUTPUT} (${buffer.length} bytes)`);
}

main().catch((err) => {
    console.error("Failed to generate icon:", err);
    process.exit(1);
});
