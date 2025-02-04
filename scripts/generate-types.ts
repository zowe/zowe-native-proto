import * as fs from "node:fs";
import * as path from "node:path";
import { execSync } from "node:child_process";

try {
    execSync("quicktype --version", { stdio: "ignore" });
} catch (error) {
    console.log("Failed to invoke quicktype. Ensure that you have installed the repo dependencies and try again.");
}

const MESSAGES_DIR = path.resolve(__dirname, "..", "messages");
const SDK_DOC_DIR = path.resolve(__dirname, "..", "packages", "sdk", "src", "doc");
const GO_DOC_DIR = path.resolve(__dirname, "..", "native", "golang", "doc");

// First generate the base interfaces
function generateBaseInterfaces(): void {
    const commonDir = path.join(SDK_DOC_DIR, "common");
    fs.mkdirSync(commonDir, { recursive: true });

    // Generate IRpcRequest
    const requestCommand = `quicktype "common/IRpcRequest.schema.json" --src-lang schema --lang typescript --just-types --acronym-style camel --top-level IRpcRequest`;
    try {
        const requestOutput = execSync(requestCommand, { cwd: MESSAGES_DIR }).toString();
        const requestContent = `// Generated from IRpcRequest.schema.json\n\n${requestOutput.replace(/^interface/, "export interface")}`;
        fs.writeFileSync(path.join(commonDir, "IRpcRequest.ts"), requestContent);
        console.log("Generated base IRpcRequest interface");
    } catch (error) {
        console.error("Error generating IRpcRequest:", error);
    }

    // Generate IRpcResponse
    const responseCommand = `quicktype "common/IRpcResponse.schema.json" --src-lang schema --lang typescript --just-types --acronym-style camel --top-level IRpcResponse`;
    try {
        const responseOutput = execSync(responseCommand, { cwd: MESSAGES_DIR }).toString();
        const responseContent = `// Generated from IRpcResponse.schema.json\n\n${responseOutput.replace(/^interface/, "export interface")}`;
        fs.writeFileSync(path.join(commonDir, "IRpcResponse.ts"), responseContent);
        console.log("Generated base IRpcResponse interface");
    } catch (error) {
        console.error("Error generating IRpcResponse:", error);
    }
}

function camelize(str: string): string {
    return str.replace(/(?:^\w|[A-Z]|\b\w|\s+)/g, (match, index) => {
        if (+match === 0) return ""; // or if (/\s+/.test(match)) for white spaces
        return index === 0 ? match.toLowerCase() : match.toUpperCase();
    });
}

function postprocessTypeScript(content: string, isRequest: boolean, commandDef: string): string {
    const addInterface = content.replace(
        /interface\s+(.+?)\s*\{/,
        `interface $1 extends ${isRequest ? "IRpcRequest" : "IRpcResponse"} {${commandDef}`,
    );

    const withoutIndexable = addInterface.replace(/\s+\[property: string\]: any;/, "");
    return withoutIndexable;
}

function processSchemaFile(schemaPath: string): void {
    // Skip common interfaces as they're handled separately
    if (schemaPath.includes(path.join("common", "IRpc"))) {
        return;
    }

    const relativePath = path.relative(MESSAGES_DIR, schemaPath);
    const fileName = path.basename(schemaPath, ".schema.json");

    const schemaDir = path.dirname(relativePath);
    const tsTargetDir = path.join(SDK_DOC_DIR, schemaDir);
    const tsTargetFile = path.join(tsTargetDir, `${fileName}.ts`);
    fs.mkdirSync(tsTargetDir, { recursive: true });

    // Convert Windows paths to forward slashes for quicktype
    const relativeSchemaPath = path.relative(MESSAGES_DIR, schemaPath).split(path.sep).join("/");

    // Generate TypeScript with imports
    const tsCommand = `quicktype "${relativeSchemaPath}" --src-lang schema --lang typescript --just-types --acronym-style camel --top-level ${fileName}`;

    try {
        const tsOutput = execSync(tsCommand, { cwd: MESSAGES_DIR }).toString();
        // Add imports and export
        const importPath = path
            .relative(path.dirname(tsTargetFile), path.join(SDK_DOC_DIR, "common"))
            .split(path.sep)
            .join("/");
        const isRequest = fileName.includes("Request");
        const commandDef = isRequest
            ? `\n\tcommand: "${camelize(path.basename(path.resolve(relativePath, "..")))}";`
            : "";
        const tsFinalContent = `// Generated from ${path.posix.normalize(relativePath)}
${isRequest ? `import type { IRpcRequest } from "${importPath}/IRpcRequest";` : `import type { IRpcResponse } from "${importPath}/IRpcResponse";`}

${postprocessTypeScript(tsOutput, isRequest, commandDef)}`;
        fs.writeFileSync(tsTargetFile, tsFinalContent);
    } catch (error) {
        console.error(`Error processing TypeScript types for ${schemaPath}:`, error);
    }

    const goTargetDir = path.join(GO_DOC_DIR, path.dirname(relativePath));
    const goTargetFile = path.join(goTargetDir, `${fileName}.go`);
    fs.mkdirSync(goTargetDir, { recursive: true });

    const goCommand = `quicktype "${relativeSchemaPath}" --src-lang schema --lang go --package doc --top-level ${fileName}`;

    try {
        const goOutput = execSync(goCommand, { cwd: MESSAGES_DIR }).toString();
        fs.writeFileSync(goTargetFile, goOutput);
    } catch (error) {
        console.error(`Error processing Go types for ${schemaPath}:`, error);
    }

    console.log("✔️ ", relativePath);
}

function walkDirsAndGenerateTypes(dir: string): void {
    const entries = fs.readdirSync(dir, { withFileTypes: true });

    for (const entry of entries) {
        const fullPath = path.join(dir, entry.name);

        if (entry.isDirectory()) {
            walkDirsAndGenerateTypes(fullPath);
        } else if (entry.isFile() && entry.name.endsWith(".schema.json")) {
            processSchemaFile(fullPath);
        }
    }
}

console.log("Starting type generation...");
process.chdir(MESSAGES_DIR);
generateBaseInterfaces();
walkDirsAndGenerateTypes(MESSAGES_DIR);
console.log("Type generation complete!");
