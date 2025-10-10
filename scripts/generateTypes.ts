/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

// generateTypes.ts - Extract TypeScript interfaces and convert to C structs
import * as fs from "node:fs";
import * as path from "node:path";
import * as ts from "typescript";

// Project paths
const SDK_TYPES_DIR = path.join(__dirname, "../packages/sdk/src/doc/rpc");
const C_SCHEMAS_DIR = path.join(__dirname, "../native/zowed/schemas");
const LICENSE_HEADER_PATH = path.join(__dirname, "../LICENSE_HEADER");

// TypeScript to validator::FieldType mapping
const VALIDATOR_TYPE_MAPPING: Record<string, string> = {
    string: "STRING",
    number: "NUMBER",
    boolean: "BOOL",
    any: "ANY",
    B64String: "STRING",
};

function isInternalType(typeName: string): boolean {
    // Internal types start with uppercase letter (e.g., UssItem, Job, Dataset)
    return typeName[0].toUpperCase() === typeName[0];
}

function stripNamespace(typeName: string): string {
    return typeName.includes(".") ? typeName.split(".").pop()! : typeName;
}

function mapTypeScriptToValidatorType(tsType: string): string {
    const isCommonType = tsType.includes("common.");
    const cleanType = isCommonType ? tsType.replace(/common\./g, "") : tsType;

    // Handle array types
    if (cleanType.endsWith("[]")) return "ARRAY";

    // Handle generic objects
    if (cleanType.includes("{") && cleanType.includes("}")) return "OBJECT";

    // Handle quoted literal types
    if (cleanType.startsWith('"') && cleanType.endsWith('"')) return "STRING";

    // Direct mapping
    if (VALIDATOR_TYPE_MAPPING[cleanType]) return VALIDATOR_TYPE_MAPPING[cleanType];

    // Generic type parameters (e.g., CommandT) - treat as STRING since they're literal string types
    if (cleanType.endsWith("T") && isInternalType(cleanType)) return "STRING";

    // Custom interface/type from our own code (common.XXX)
    if (isCommonType && isInternalType(cleanType)) return "OBJECT";

    // External types (Readable, Writable, etc.) or unknown types
    return "ANY";
}

function getArrayElementType(tsType: string): string | null {
    if (!tsType.endsWith("[]")) return null;

    const arrayBaseType = tsType.slice(0, -2);
    const isCommonType = arrayBaseType.includes("common.");

    // For common types, check if it's an internal object type
    if (isCommonType) {
        const cleanType = stripNamespace(arrayBaseType);
        // Internal types (e.g., UssItem, Job, Dataset) should be OBJECT
        if (isInternalType(cleanType)) return "OBJECT";
    }

    return mapTypeScriptToValidatorType(arrayBaseType);
}

interface ExtractedInterface {
    name: string;
    properties: Array<{
        name: string;
        type: string;
        optional: boolean;
        isLiteral?: boolean;
    }>;
    extends?: string[];
    sourceFile: string;
}

function extractInterfaces(filePath: string): ExtractedInterface[] {
    const program = ts.createProgram([filePath], {
        target: ts.ScriptTarget.ES2020,
        module: ts.ModuleKind.CommonJS,
    });
    const sourceFile = program.getSourceFile(filePath);

    if (!sourceFile) {
        console.warn(`Could not parse file: ${filePath}`);
        return [];
    }

    const interfaces: ExtractedInterface[] = [];
    const fileName = path.basename(filePath, ".ts");

    function visit(node: ts.Node) {
        if (ts.isInterfaceDeclaration(node)) {
            const interfaceName = node.name.text;
            const properties: ExtractedInterface["properties"] = [];

            // Extract heritage clauses (extends)
            const extendsClause = node.heritageClauses?.find((clause) => clause.token === ts.SyntaxKind.ExtendsKeyword);
            const extendsTypes = extendsClause?.types.map((type) => type.expression.getText(sourceFile)) || [];

            // Extract properties
            for (const member of node.members) {
                if (ts.isPropertySignature(member) && member.name) {
                    const propName = member.name.getText(sourceFile);
                    const propType = member.type?.getText(sourceFile) || "any";
                    const optional = !!member.questionToken;

                    // Check if this is a literal type (e.g., "listDatasets", "2.0") or generic type parameter (e.g., CommandT)
                    const isStringLiteral = propType.startsWith('"') && propType.endsWith('"');
                    const isGenericParam = propType.endsWith("T") && isInternalType(propType);
                    const isLiteralType = isStringLiteral || isGenericParam;

                    properties.push({
                        name: propName,
                        type: propType,
                        optional,
                        isLiteral: isLiteralType,
                    });
                }
            }

            interfaces.push({
                name: interfaceName,
                properties,
                extends: extendsTypes.length > 0 ? extendsTypes : undefined,
                sourceFile: fileName,
            });
        } else if (ts.isTypeAliasDeclaration(node) && node.type) {
            // Handle type aliases: export type CreateMemberResponse = common.CommandResponse;
            const aliasName = node.name.text;
            const aliasedType = node.type.getText(sourceFile);
            const baseTypeName = stripNamespace(aliasedType);

            // For CommandResponse aliases, add the success field directly
            const properties: ExtractedInterface["properties"] =
                baseTypeName === "CommandResponse" ? [{ name: "success", type: "boolean", optional: false }] : [];

            interfaces.push({
                name: aliasName,
                properties,
                extends: baseTypeName === "CommandResponse" ? undefined : [baseTypeName],
                sourceFile: fileName,
            });
        }

        ts.forEachChild(node, visit);
    }

    visit(sourceFile);
    return interfaces;
}

// Dynamically collected base class properties and interfaces
const baseClassProperties: Map<string, Set<string>> = new Map();
const allInterfaces: Map<string, ExtractedInterface> = new Map();

// License header content
let licenseHeader: string = "";

function loadLicenseHeader(): void {
    try {
        licenseHeader = fs.readFileSync(LICENSE_HEADER_PATH, "utf8");
        // Ensure the license header ends with a newline
        if (!licenseHeader.endsWith("\n")) {
            licenseHeader += "\n";
        }
    } catch (error) {
        console.warn(`Could not load license header from ${LICENSE_HEADER_PATH}:`, error);
        licenseHeader = "";
    }
}

function collectAllProperties(iface: ExtractedInterface): Array<ExtractedInterface["properties"][0]> {
    const allProps: Array<ExtractedInterface["properties"][0]> = [];
    const fieldsSeen = new Set<string>();

    // Add inherited properties from base classes
    if (iface.extends) {
        for (const baseType of iface.extends) {
            const baseName = stripNamespace(baseType);
            const baseInterface = allInterfaces.get(baseName);
            if (baseInterface) {
                for (const baseProp of baseInterface.properties) {
                    if (!fieldsSeen.has(baseProp.name)) {
                        allProps.push(baseProp);
                        fieldsSeen.add(baseProp.name);
                    }
                }
            }
        }
    }

    // Add own properties
    for (const prop of iface.properties) {
        if (!fieldsSeen.has(prop.name)) {
            allProps.push(prop);
            fieldsSeen.add(prop.name);
        }
    }

    return allProps;
}

function generateSchemaFields(iface: ExtractedInterface): string[] {
    const schemaFields: string[] = [];
    const allProps = collectAllProperties(iface);

    for (const prop of allProps) {
        // Skip literal type fields (like "command" with value "listDatasets")
        if (prop.isLiteral) continue;

        const arrayElementType = getArrayElementType(prop.type);
        const fieldType = arrayElementType || mapTypeScriptToValidatorType(prop.type);
        const requirement = prop.optional ? "OPTIONAL" : "REQUIRED";
        const macroSuffix = arrayElementType ? "_ARRAY" : "";

        schemaFields.push(`FIELD_${requirement}${macroSuffix}(${prop.name}, ${fieldType})`);
    }

    return schemaFields;
}

function generateHeaderFile(interfaces: ExtractedInterface[], fileName: string): string {
    const headerGuard = `${fileName.toUpperCase()}_HPP`;
    let content = "";

    // Add license header if available
    if (licenseHeader) {
        content += `${licenseHeader}\n`;
    }

    // Add auto-generation disclaimer
    content += `// Code generated by generateTypes.ts. DO NOT EDIT.\n\n`;

    content += `#ifndef ${headerGuard}\n#define ${headerGuard}\n\n`;
    content += `#include "../validator.hpp"\n\n`;

    // Generate schemas in the same order as the source file
    for (const iface of interfaces) {
        const schemaFields = generateSchemaFields(iface);
        if (schemaFields.length > 0) {
            content += `struct ${iface.name} {};\n`;
            content += `ZJSON_SCHEMA(${iface.name},\n`;
            content += `    ${schemaFields.join(",\n    ")}\n`;
            content += `);\n\n`;
        }
    }

    content += `#endif\n`;
    return content;
}

function collectBaseClassProperties(interfaces: ExtractedInterface[]): void {
    // First pass: store all interfaces and their properties
    for (const iface of interfaces) {
        const props = new Set(iface.properties.map((p) => p.name));
        baseClassProperties.set(iface.name, props);
        allInterfaces.set(iface.name, iface);
    }

    // Second pass: add inherited properties to base classes
    for (const iface of interfaces) {
        if (!iface.extends) continue;

        const currentProps = baseClassProperties.get(iface.name)!;
        for (const baseType of iface.extends) {
            const baseName = stripNamespace(baseType);
            const baseProps = baseClassProperties.get(baseName);
            if (baseProps) {
                baseProps.forEach((prop) => currentProps.add(prop));
            }
        }
    }
}

function processAllRpcFiles(): void {
    // Load the license header
    loadLicenseHeader();

    if (!fs.existsSync(C_SCHEMAS_DIR)) {
        fs.mkdirSync(C_SCHEMAS_DIR, { recursive: true });
    }

    // Count all TypeScript files (excluding index.ts)
    const allTsFiles = fs.readdirSync(SDK_TYPES_DIR).filter((file) => file.endsWith(".ts") && file !== "index.ts");
    console.log(`Processing ${allTsFiles.length} TypeScript files...`);

    // First extract common.ts to get base types like CommandResponse, Job, etc.
    const commonFilePath = path.join(SDK_TYPES_DIR, "common.ts");
    if (fs.existsSync(commonFilePath)) {
        console.log(`Extracting interfaces from common.ts...`);
        const commonInterfaces = extractInterfaces(commonFilePath);
        // Build base class map with common types first
        collectBaseClassProperties(commonInterfaces);
    }

    // Extract all interfaces from all files
    const allInterfaces: ExtractedInterface[] = [];
    const requests: ExtractedInterface[] = [];
    const responses: ExtractedInterface[] = [];

    for (const file of allTsFiles) {
        const filePath = path.join(SDK_TYPES_DIR, file);
        if (filePath === commonFilePath) {
            continue;
        }

        console.log(`Extracting interfaces from ${file}...`);
        const interfaces = extractInterfaces(filePath);
        allInterfaces.push(...interfaces);

        // Separate into requests and responses
        for (const iface of interfaces) {
            if (iface.name.endsWith("Request")) {
                requests.push(iface);
            } else if (iface.name.endsWith("Response")) {
                responses.push(iface);
            }
        }
    }

    // Build the base class property map (re-build with all interfaces)
    collectBaseClassProperties(allInterfaces);

    // Generate requests.hpp
    if (requests.length > 0) {
        const requestsContent = generateHeaderFile(requests, "requests");
        const requestsPath = path.join(C_SCHEMAS_DIR, "requests.hpp");
        fs.writeFileSync(requestsPath, requestsContent);
        console.log(`Generated requests.hpp with ${requests.length} request schemas`);
    }

    // Generate responses.hpp
    if (responses.length > 0) {
        const responsesContent = generateHeaderFile(responses, "responses");
        const responsesPath = path.join(C_SCHEMAS_DIR, "responses.hpp");
        fs.writeFileSync(responsesPath, responsesContent);
        console.log(`Generated responses.hpp with ${responses.length} response schemas`);
    }
}

// Run the conversion
if (require.main === module) {
    try {
        processAllRpcFiles();
        console.log("Type conversion completed successfully!");
    } catch (error) {
        console.error("Error during type conversion:", error);
        process.exit(1);
    }
}
