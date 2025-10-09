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

// convertTypes.ts - Extract TypeScript interfaces and convert to C structs
import * as ts from "typescript";
import * as fs from "fs";
import * as path from "path";

// Project paths
const SDK_GEN_DIR = path.join(__dirname, "../packages/sdk/src/doc/rpc");
const C_SCHEMAS_DIR = path.join(__dirname, "../native/zowed/schemas");
const LICENSE_HEADER_PATH = path.join(__dirname, "../LICENSE_HEADER");

// TypeScript to validator::FieldType mapping
const VALIDATOR_TYPE_MAPPING: Record<string, string> = {
    string: "STRING",
    number: "NUMBER",
    boolean: "BOOL",
    bool: "BOOL",
    any: "ANY",
    B64String: "STRING",
};

function mapTypeScriptToValidatorType(tsType: string): string {
    let cleanType = tsType;
    const isCommonType = cleanType.includes("common.");

    if (isCommonType) {
        cleanType = cleanType.replace(/common\./g, "");
    }

    // Handle array types
    if (cleanType.endsWith("[]")) {
        return "ARRAY";
    }
    // Handle number with int comment
    else if (cleanType.includes("number") && cleanType.includes("int")) {
        return "INTEGER";
    }
    // Handle generic objects
    else if (cleanType.includes("{") && cleanType.includes("}")) {
        return "OBJECT";
    }
    // Handle quoted literal types
    else if (cleanType.startsWith('"') && cleanType.endsWith('"')) {
        return "STRING";
    }
    // Direct mapping
    else if (VALIDATOR_TYPE_MAPPING[cleanType]) {
        return VALIDATOR_TYPE_MAPPING[cleanType];
    }
    // Generic type parameters (e.g., CommandT) - treat as STRING since they're literal string types
    else if (cleanType.endsWith("T") && cleanType[0].toUpperCase() === cleanType[0]) {
        return "STRING";
    }
    // Custom interface/type from our own code (common.XXX)
    else if (isCommonType && cleanType[0].toUpperCase() === cleanType[0]) {
        return "OBJECT";
    }
    // External types (Readable, Writable, etc.) or unknown types
    else {
        return "ANY";
    }
}

function getArrayElementType(tsType: string): string | null {
    let cleanType = tsType;
    if (cleanType.includes("common.")) {
        cleanType = cleanType.replace(/common\./g, "");
    }

    if (cleanType.endsWith("[]")) {
        const arrayBaseType = cleanType.slice(0, -2);
        return mapTypeScriptToValidatorType(arrayBaseType);
    }
    return null;
}

interface ExtractedInterface {
    name: string;
    properties: Array<{
        name: string;
        type: string;
        optional: boolean;
        comment?: string;
        hasTypeOverride?: boolean;
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
    const checker = program.getTypeChecker();
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
            const type = checker.getTypeAtLocation(node);
            const properties: ExtractedInterface["properties"] = [];

            // Extract heritage clauses (extends)
            const extendsClause = node.heritageClauses?.find((clause) => clause.token === ts.SyntaxKind.ExtendsKeyword);
            const extendsTypes = extendsClause?.types.map((type) => type.expression.getText()) || [];

            // Extract properties
            for (const member of node.members) {
                if (ts.isPropertySignature(member) && member.name) {
                    const propName = member.name.getText();
                    const propType = member.type?.getText() || "any";
                    const optional = !!member.questionToken;

                    // Check if this is a literal type (e.g., "listDatasets", 'test', false, 123, "2.0")
                    const isLiteralType =
                        (propType.startsWith('"') && propType.endsWith('"')) ||
                        (propType.startsWith("'") && propType.endsWith("'")) ||
                        propType === "true" ||
                        propType === "false" ||
                        /^\d+$/.test(propType); // numeric literals

                    // Extract comments within the property signature line
                    const memberStart = member.getFullStart();
                    const memberEnd = member.getEnd();
                    const memberText = sourceFile.text.substring(memberStart, memberEnd);

                    let comment: string | undefined;
                    let nameOverride: string | undefined;
                    let typeOverride: string | undefined;

                    // Look for inline comments /* ... */ within the property line
                    const inlineCommentMatch = memberText.match(/\/\*\s*(.*?)\s*\*\//);
                    if (inlineCommentMatch) {
                        comment = inlineCommentMatch[1].trim();

                        // Check for override syntax: name:type, :type, or name:
                        // Split only on the first colon to handle types like zjson::Value
                        const colonIndex = comment.indexOf(":");
                        if (colonIndex !== -1) {
                            const nameMatch = comment.substring(0, colonIndex);
                            const typeMatch = comment.substring(colonIndex + 1);
                            nameOverride = nameMatch.trim() || undefined;
                            typeOverride = typeMatch.trim() || undefined;
                            comment = undefined; // Remove the override syntax from comment
                        }

                        // Only keep non-empty comments that aren't overrides
                        if (!comment) {
                            comment = undefined;
                        }
                    }

                    // Use override values if present, otherwise use extracted values
                    const finalName = nameOverride || propName;
                    const finalType = typeOverride || propType;
                    const hasTypeOverride = typeOverride !== undefined;

                    properties.push({
                        name: finalName,
                        type: finalType,
                        optional,
                        comment,
                        hasTypeOverride,
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
            const aliasedType = node.type.getText();

            // Extract the base type name (remove namespace prefixes like "common.")
            const baseTypeName = aliasedType.includes(".") ? aliasedType.split(".").pop()! : aliasedType;

            // For CommandResponse aliases, add the success field directly
            const properties: ExtractedInterface["properties"] =
                baseTypeName === "CommandResponse" ? [{ name: "success", type: "boolean", optional: false }] : [];

            // Create a new interface with the alias name
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
let baseClassProperties: Map<string, Set<string>> = new Map();
let allInterfaces: Map<string, ExtractedInterface> = new Map();

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

function generateSchemaFields(iface: ExtractedInterface): string[] {
    const schemaFields: string[] = [];

    // Collect all properties (including inherited from base classes)
    const allProps: Array<(typeof iface.properties)[0]> = [];
    const fieldsSeen = new Set<string>();

    // Add inherited properties from base classes
    if (iface.extends && iface.extends.length > 0) {
        for (const baseType of iface.extends) {
            const baseName = baseType.includes(".") ? baseType.split(".").pop()! : baseType;
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

    // Generate field descriptors
    for (const prop of allProps) {
        // Skip literal type fields
        if (prop.isLiteral && !prop.hasTypeOverride) {
            continue;
        }

        // Skip the "command" field - it's a string literal representing the RPC method name
        // This comes from CommandRequest<T> where T is a literal like "listDatasets"
        if (prop.name === "command") {
            continue;
        }

        const fieldType = mapTypeScriptToValidatorType(prop.type);
        const arrayElementType = getArrayElementType(prop.type);
        const requirement = prop.optional ? "OPTIONAL" : "REQUIRED";

        if (arrayElementType) {
            // Array field with element type
            schemaFields.push(`FIELD_${requirement}_ARRAY(${prop.name}, ${arrayElementType})`);
        } else {
            // Regular field
            schemaFields.push(`FIELD_${requirement}(${prop.name}, ${fieldType})`);
        }
    }

    return schemaFields;
}

function generateHeaderFile(interfaces: ExtractedInterface[], fileName: string): string {
    const headerGuard = `${fileName.toUpperCase()}_HPP`;
    let content = "";

    // Add license header if available
    if (licenseHeader) {
        content += licenseHeader + "\n";
    }

    // Add auto-generation disclaimer
    content += `// Code generated by convertTypes.ts. DO NOT EDIT.\n\n`;

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
    // First pass: collect all interface properties and store interfaces
    for (const iface of interfaces) {
        const props = new Set<string>();
        for (const prop of iface.properties) {
            props.add(prop.name);
        }
        baseClassProperties.set(iface.name, props);
        allInterfaces.set(iface.name, iface);
    }

    // Second pass: add inherited properties to base classes
    for (const iface of interfaces) {
        if (iface.extends && iface.extends.length > 0) {
            const currentProps = baseClassProperties.get(iface.name)!;

            for (const baseType of iface.extends) {
                const baseName = baseType.includes(".") ? baseType.split(".").pop()! : baseType;
                const baseProps = baseClassProperties.get(baseName);
                if (baseProps) {
                    baseProps.forEach((prop) => currentProps.add(prop));
                }
            }
        }
    }
}

function processAllGenFiles(): void {
    // Load the license header
    loadLicenseHeader();

    if (!fs.existsSync(C_SCHEMAS_DIR)) {
        fs.mkdirSync(C_SCHEMAS_DIR, { recursive: true });
    }

    // Count all TypeScript files (excluding index.ts)
    const allTsFiles = fs.readdirSync(SDK_GEN_DIR).filter((file) => file.endsWith(".ts") && file !== "index.ts");
    console.log(`Processing ${allTsFiles.length} TypeScript files...`);

    // First extract common.ts to get base types like CommandResponse, Job, etc.
    const commonFilePath = path.join(SDK_GEN_DIR, "common.ts");
    if (fs.existsSync(commonFilePath)) {
        console.log(`Extracting interfaces from common.ts...`);
        const commonInterfaces = extractInterfaces(commonFilePath);
        // Build base class map with common types first
        collectBaseClassProperties(commonInterfaces);
    }

    // Process TypeScript files (skip common.ts and index.ts)
    const genFiles = fs
        .readdirSync(SDK_GEN_DIR)
        .filter((file) => file.endsWith(".ts") && file !== "common.ts" && file !== "index.ts");

    // Extract all interfaces from all files
    const allInterfaces: ExtractedInterface[] = [];
    const requests: ExtractedInterface[] = [];
    const responses: ExtractedInterface[] = [];

    for (const file of genFiles) {
        const filePath = path.join(SDK_GEN_DIR, file);
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
        processAllGenFiles();
        console.log("Type conversion completed successfully!");
    } catch (error) {
        console.error("Error during type conversion:", error);
        process.exit(1);
    }
}
